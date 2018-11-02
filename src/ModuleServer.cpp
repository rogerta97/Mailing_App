#include "ModuleServer.h"
#include "Log.h"
#include "imgui/imgui.h"
#include "serialization/PacketTypes.h"
#include "database/MySqlDatabaseGateway.h"
#include "database/SimulatedDatabaseGateway.h"
#include "Application.h"

static bool g_SimulateDatabaseConnection = false;


#define HEADER_SIZE sizeof(uint32_t)
#define RECV_CHUNK_SIZE 4096


ModuleServer::ModuleServer()
{
	mysqlDatabaseGateway = new MySqlDatabaseGateway();
	simulatedDatabaseGateway = new SimulatedDatabaseGateway();
}

ModuleServer::~ModuleServer()
{
	delete mysqlDatabaseGateway;
	delete simulatedDatabaseGateway;
}

bool ModuleServer::update()
{
	if (draw_config)
		DrawConnectionConfig();

	switch (state)
	{
	case ServerState::Starting:
		startServer();
		break;
	case ServerState::Running:
		handleIncomingData();
		handleOutgoingData();
		deleteInvalidSockets();
		break;
	case ServerState::Stopping:
		stopServer();
		break;
	default:
		break;
	}

	return true;
}

bool ModuleServer::cleanUp()
{
	stopServer();

	return true;
}

void ModuleServer::onPacketReceived(SOCKET socket, const InputMemoryStream & stream)
{
	PacketType packetType;

	int packet_t = -1;
	stream.Read<int>(packet_t);

	packetType = (PacketType)packet_t;

	switch (packetType)
	{
	case PacketType::LoginRequest:
		onPacketReceivedLogin(socket, stream);
		LOG("onPacketReceived() - packetType: LoginRequest");
		break;
	case PacketType::QueryAllMessagesRequest:
		onPacketReceivedQueryAllMessages(socket, stream);
		LOG("onPacketReceived() - packetType: QueryAllMessagesRequest");
		break;
	case PacketType::SendMessageRequest:
		onPacketReceivedSendMessage(socket, stream);
		LOG("onPacketReceived() - packetType: SendMessageRequest");
		break;
	case PacketType::ConnectedPing:
		onPacketReceivedConnectionPing(socket, stream);
		LOG("onPacketReceived() - packetType: ConnectedPing");
		break;
	case PacketType::WritingPing:
		onPacketReceivedWritingPing(socket, stream);
		LOG("onPacketReceived() - packetType: WritingPing");
		break; 
	case PacketType::AllUsersRequest:
		onPacketReceivedQueryAllUsers(socket, stream);
		LOG("onPacketReceived() - packetType: AllUsersRequest");
		break;
	default:
		LOG("Unknown packet type received");
		break;
	}
}

void ModuleServer::onPacketReceivedLogin(SOCKET socket, const InputMemoryStream & stream)
{
	std::string loginName;

	stream.Read(loginName);

	ClientStateInfo & client = getClientStateInfoForSocket(socket);
	client.loginName = loginName;
}

void ModuleServer::onPacketReceivedQueryAllMessages(SOCKET socket, const InputMemoryStream & stream)
{
	// Get the username of this socket and send the response to it
	ClientStateInfo & clientStateInfo = getClientStateInfoForSocket(socket);
	sendPacketQueryAllMessagesResponse(socket, clientStateInfo.loginName);
}

void ModuleServer::onPacketReceivedConnectionPing(SOCKET socket, const InputMemoryStream & stream)
{
	ClientStateInfo & clientStateInfo = getClientStateInfoForSocket(socket);
	database()->sendConnectedPing(clientStateInfo.loginName);
}

void ModuleServer::onPacketReceivedWritingPing(SOCKET socket, const InputMemoryStream & stream)
{
	ClientStateInfo & clientStateInfo = getClientStateInfoForSocket(socket);
	database()->sendWritingPing(clientStateInfo.loginName);
}


void ModuleServer::onPacketReceivedQueryAllUsers(SOCKET socket, const InputMemoryStream& stream)
{
	sendPacketQueryAllUsersResponse(socket);
}


void ModuleServer::sendPacketQueryAllUsersResponse(SOCKET socket)
{
	std::vector<User> users = database()->getAllUsers();

	OutputMemoryStream outStream;
	outStream.Write<int>((int)PacketType::AllUsersResponse);
	outStream.Write<uint32_t>((uint32_t)users.size());
	for (int i = 0; i < users.size(); i++)
	{
		outStream.Write(users[i].username);
		outStream.Write(users[i].password);
		outStream.Write(App->DateTimeToString(users[i].last_connected));
		outStream.Write(App->DateTimeToString(users[i].last_writing));
	}

	sendPacket(socket, outStream);
}

void ModuleServer::sendPacketQueryAllMessagesResponse(SOCKET socket, const std::string &username)
{
	// Obtain the list of messages from the DB
	std::vector<Message> messages = database()->getAllMessagesReceivedByUser(username);
	
	OutputMemoryStream outStream;
	outStream.Write<int>((int)PacketType::QueryAllMessagesResponse);
	outStream.Write<uint32_t>((uint32_t)messages.size());
	for (int i = 0; i < messages.size(); i++)
	{
		outStream.Write(messages[i].senderUsername);
		outStream.Write(messages[i].receiverUsername);
		outStream.Write(messages[i].subject);
		outStream.Write(messages[i].body);
	}

	sendPacket(socket, outStream);

}

void ModuleServer::onPacketReceivedSendMessage(SOCKET socket, const InputMemoryStream & stream)
{
	Message message;
	stream.Read(message.senderUsername);
	stream.Read(message.receiverUsername);
	stream.Read(message.body);

	// Insert the message in the database
	database()->insertMessage(message);
}

void ModuleServer::sendPacket(SOCKET socket, OutputMemoryStream & stream)
{
	ClientStateInfo & client = getClientStateInfoForSocket(socket);
	// Copy the packet into the send buffer
	size_t oldSize = client.sendBuffer.size();
	client.sendBuffer.resize(oldSize + HEADER_SIZE + stream.GetSize());
	uint32_t &packetSize = *(uint32_t*)&client.sendBuffer[oldSize];
	packetSize = HEADER_SIZE + stream.GetSize(); // header size + payload size
	//std::copy(stream.GetBufferPtr(), stream.GetBufferPtr() + stream.GetSize(), &client.sendBuffer[oldSize] + HEADER_SIZE);
	memcpy(&client.sendBuffer[oldSize] + HEADER_SIZE, stream.GetBufferPtr(), stream.GetSize());
}



// GUI: Modify this to add extra features...

void ModuleServer::DrawConnectionConfig()
{
	ImGui::Begin("Connection configuration", &draw_config);

	// Port
	ImGui::InputInt("Port", &port);

	// Simulate database
	ImGui::Checkbox("Simulate database", &g_SimulateDatabaseConnection);

	// IP address
	ImGui::InputText("IP", serverIP, sizeof(*serverIP));

	ImGui::Text("Connected clients:");
	for (auto & client : clients)
		ImGui::Text(" - %s", client.loginName.c_str());

	database()->updateGUI();

	ImGui::End();
}


// Low-level networking stuff

void ModuleServer::startServer()
{
	// Create socket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket()");
	}

	// Force reuse address
	int enable = 1;
	int res = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&enable, sizeof(int));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("setsockopt() SO_REUSEADDR");
	}

	// Bind
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	res = bind(listenSocket, (const sockaddr*)&bindAddr, sizeof(bindAddr));
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("bind()");
	}

	// Listen
	res = listen(listenSocket, 32);
	if (res == SOCKET_ERROR)
	{
		printWSErrorAndExit("listen()");
	}

	// Next state
	state = ServerState::Running;

	LOG("Sever listening port %d", port);
}

void ModuleServer::stopServer()
{
	for (auto & clientInfo : clients) {
		closesocket(clientInfo.socket);
	}

	clients.clear();

	closesocket(listenSocket);

	state = ServerState::Off;

	LOG("Server off");
}

void ModuleServer::handleIncomingData()
{
	std::vector<SOCKET> allSockets = getAllSockets();
	std::vector<SOCKET> readableSockets = selectReadableSockets(allSockets);

	for (auto& socket : readableSockets)
	{
		if (socket == listenSocket)
		{
			sockaddr_in clientAddr;
			int addrLen = sizeof(clientAddr);
			SOCKET connectedSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
			if (connectedSocket == INVALID_SOCKET)
			{
				printWSErrorAndExit("accept()");
			}
			createClientStateInfoForSocket(connectedSocket);
			LOG("New connection accepted");
		}
		else
		{
			ClientStateInfo & clientStateInfo = getClientStateInfoForSocket(socket);
			handleIncomingDataFromClient(clientStateInfo);
		}
	}
}

void ModuleServer::handleOutgoingData()
{
	std::vector<SOCKET> allSockets = getAllSockets();
	std::vector<SOCKET> writableSockets = selectWritableSockets(allSockets);

	for (auto& socket : writableSockets)
	{
		ClientStateInfo & clientStateInfo = getClientStateInfoForSocket(socket);
		handleOutgoingDataToClient(clientStateInfo);
	}
}

void ModuleServer::handleIncomingDataFromClient(ClientStateInfo & info)
{
	if (info.recvBuffer.size() - info.recvByteHead < RECV_CHUNK_SIZE) {
		info.recvBuffer.resize(info.recvByteHead + RECV_CHUNK_SIZE);
	}

	int res = recv(info.socket, (char*)&info.recvBuffer[info.recvByteHead], RECV_CHUNK_SIZE, 0);
	if (res == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// Do nothing
		}
		else
		{
			printWSError("recv() - Error: Disconnecting client");
			LOG("recv() - Error: Disconnecting client: %s", info.loginName.c_str());
			info.invalid = true;
		}
	}
	else
	{
		if (res == 0)
		{
			LOG("Client disconnected flawlessly - client: %s", info.loginName.c_str());
			info.invalid = true;
			return;
		}

		info.recvByteHead += res;
		while (info.recvByteHead - info.recvPacketHead > HEADER_SIZE)
		{
			const size_t recvWindow = info.recvByteHead - info.recvPacketHead;
			const uint32_t packetSize = *(uint32_t*)&info.recvBuffer[info.recvPacketHead];
			if (recvWindow >= packetSize)
			{
				InputMemoryStream stream(packetSize - HEADER_SIZE);
				//std::copy(&info.recvBuffer[info.recvPacketHead + HEADER_SIZE], &info.recvBuffer[info.recvPacketHead + packetSize], (uint8_t*)stream.GetBufferPtr());
				memcpy(stream.GetBufferPtr(), &info.recvBuffer[info.recvPacketHead + HEADER_SIZE], packetSize - HEADER_SIZE);
				onPacketReceived(info.socket, stream);
				info.recvPacketHead += packetSize;
			}
		}
		if (info.recvPacketHead >= info.recvByteHead)
		{
			info.recvPacketHead = 0;
			info.recvByteHead = 0;
		}
	}
}

void ModuleServer::handleOutgoingDataToClient(ClientStateInfo & info)
{
	if (info.sendHead < info.sendBuffer.size())
	{
		int res = send(info.socket, (const char *)&info.sendBuffer[info.sendHead], (int)info.sendBuffer.size(), 0);
		if (res == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// Do nothing
			}
			else
			{
				printWSError("send() - Error: Disconnecting client");
				LOG("send() - Error: Disconnecting client: %s", info.loginName.c_str());
				info.invalid = true;
			}
		}
		else
		{
			info.sendHead += res;
		}

		if (info.sendHead >= info.sendBuffer.size())
		{
			info.sendHead = 0;
			info.sendBuffer.clear();
		}
	}
}

std::vector<SOCKET> ModuleServer::getAllSockets() const
{
	std::vector<SOCKET> allSockets;
	allSockets.push_back(listenSocket);
	for (auto& client : clients)
	{
		allSockets.push_back(client.socket);
	}
	return allSockets;
}

void ModuleServer::createClientStateInfoForSocket(SOCKET s)
{
	assert(!existsClientStateInfoForSocket(s) && "Cannot create more than one client per socket");
	ClientStateInfo clientStateInfo;
	clientStateInfo.socket = s;
	clientStateInfo.loginName = "<pending login>";
	clients.emplace_back(clientStateInfo);
}

ModuleServer::ClientStateInfo & ModuleServer::getClientStateInfoForSocket(SOCKET s)
{
	for (auto& clientStateInfo : clients)
	{
		if (s == clientStateInfo.socket)
		{
			return clientStateInfo;
		}
	}

	//assert(nullptr && "The client for this socket does not exist.");
}

bool ModuleServer::existsClientStateInfoForSocket(SOCKET s)
{
	for (auto& clientStateInfo : clients)
	{
		if (s == clientStateInfo.socket)
		{
			return true;
		}
	}
	return false;
}

void ModuleServer::deleteInvalidSockets()
{
	auto it = clients.begin();
	while (it != clients.end())
	{
		if (it->invalid)
		{
			closesocket(it->socket);
			it = clients.erase(it);
		}
		else
		{
			++it;
		}
	}
}

IDatabaseGateway * ModuleServer::database()
{
	if (g_SimulateDatabaseConnection) {
		return simulatedDatabaseGateway;
	} else {
		return mysqlDatabaseGateway;
	}
}
