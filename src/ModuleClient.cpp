#include "ModuleClient.h"
#include "Log.h"
#include "imgui/imgui.h"
#include "serialization/PacketTypes.h"
#include "ModuleMainMenu.h"
#include "Application.h"
#include "ModuleServer.h"
#include "database\IDatabaseGateway.h"

#define HEADER_SIZE sizeof(uint32_t)
#define RECV_CHUNK_SIZE 4096

bool ModuleClient::update()
{
	updateGUI();

	switch (state)
	{
	case ClientState::Connecting:
		connectToServer();
		break;
	case ClientState::Connected:
		handleIncomingData();
		updateMessenger();
		handleOutgoingData();
		break;
	case ClientState::Disconnecting:
		disconnectFromServer();
		break;
	default:
		break;
	}

	return true;
}

bool ModuleClient::cleanUp()
{
	disconnectFromServer();
	return true;
}

void ModuleClient::updateMessenger()
{
	switch (messengerState)
	{
	case MessengerState::SendingLogin:
		sendPacketLogin(senderBuf.c_str());
		break;
	case MessengerState::RequestingMessages:
		// Idle, do nothing
		break;
	case MessengerState::ReceivingMessages:
		// Idle, do nothing
		break;
	case MessengerState::ShowingMessages:
		// Idle, do nothing
		break;
	case MessengerState::ComposingMessage:
		// Idle, do nothing
		break;
	case MessengerState::SendingMessage:
		sendPacketSendMessage(receiverBuf, messageBuf);
		break;
	default:
		break;
	}
}

void ModuleClient::onPacketReceived(const InputMemoryStream & stream)
{
	PacketType packetType;

	int packet_t = -1;
	stream.Read<int>(packet_t);

	packetType = (PacketType)packet_t;
	
	switch (packetType)
	{
	case PacketType::QueryAllMessagesResponse:
		onPacketReceivedQueryAllMessagesResponse(stream);
		LOG("onPacketReceived() - packetType: QueryAllMessagesResponse");
		break;
	case PacketType::AllUsersResponse:
		onPacketReceivedAllUsersResponse(stream);
		LOG("onPacketReceived() - packetType: AllUsersResponse");
		break;
	default:
		LOG("Unknown packet type received");
		break;
	}
}

void ModuleClient::onPacketReceivedQueryAllMessagesResponse(const InputMemoryStream & stream)
{
	messages.clear();

	uint32_t messageCount;
	stream.Read<uint32_t>(messageCount);
	std::string sent_time;
	for (int i = 0; i < messageCount; i++)
	{
		Message m;
		stream.Read(m.senderUsername);
		stream.Read(m.receiverUsername);
		stream.Read(m.body);
		stream.Read(sent_time);
		m.sent_time = App->StringToDateTime(sent_time);
		stream.Read<bool>(m.is_read);
		stream.Read<bool>(m.is_received);
		messages.push_back(m);
	}

	messengerState = MessengerState::ShowingMessages;
}


void ModuleClient::onPacketReceivedAllUsersResponse(const InputMemoryStream &stream)
{
	current_users.clear();

	uint32_t user_num;
	stream.Read<uint32_t>(user_num);
	std::string connected_time;
	std::string writing_time;
	for (int i = 0; i < user_num; i++)
	{
		User m;
		stream.Read(m.username);
		stream.Read(m.password);
		stream.Read(connected_time);
		stream.Read(writing_time);
		m.last_connected = App->StringToDateTime(connected_time);
		m.last_writing = App->StringToDateTime(writing_time);
		current_users.push_back(m);
	}
}

void ModuleClient::sendPacketLogin(const char * username)
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::LoginRequest);
	stream.Write(std::string(username));
	sendPacket(stream);

	messengerState = MessengerState::RequestingMessages;
}

void ModuleClient::sendPacketMessagesRead(const char *sender)
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::MessagesRead);
	stream.Write(std::string(sender));
	stream.Write(std::string(senderBuf));

	sendPacket(stream);
}

void ModuleClient::sendPacketQueryMessages(const char *sender)
{
	OutputMemoryStream stream;
	
	stream.Write<int>((int)PacketType::QueryAllMessagesRequest);
	stream.Write(std::string(sender));
	sendPacket(stream);

	messengerState = MessengerState::ReceivingMessages;
}


void ModuleClient::sendPacketConnectedPing()
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::ConnectedPing);
	sendPacket(stream);
}

void ModuleClient::sendPacketWritingPing()
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::WritingPing);
	sendPacket(stream);
}

void ModuleClient::sendPacketUsersRequest()
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::AllUsersRequest);
	sendPacket(stream);
}



void ModuleClient::sendPacketSendMessage(const char * receiver,const char *message)
{
	OutputMemoryStream stream;

	stream.Write<int>((int)PacketType::SendMessageRequest);
	stream.Write(std::string(senderBuf));
	stream.Write(std::string(receiver));
	stream.Write(std::string(message));
	sendPacket(stream);

	messengerState = MessengerState::RequestingMessages;
}

// This function is done for you: Takes the stream and schedules its internal buffer to be sent
void ModuleClient::sendPacket(const OutputMemoryStream & stream)
{
	// Copy the packet into the send buffer
	size_t oldSize = sendBuffer.size();
	sendBuffer.resize(oldSize + HEADER_SIZE + stream.GetSize());
	uint32_t &packetSize = *(uint32_t*)&sendBuffer[oldSize];
	packetSize = HEADER_SIZE + stream.GetSize(); // header size + payload size
	//std::copy(stream.GetBufferPtr(), stream.GetBufferPtr() + stream.GetSize(), &sendBuffer[oldSize] + HEADER_SIZE);
	memcpy(&sendBuffer[oldSize] + HEADER_SIZE, stream.GetBufferPtr(), stream.GetSize());
}


// GUI: Modify this to add extra features...

void ModuleClient::updateGUI()
{
	ImGui::Begin("Client Window");

	if (state == ClientState::Disconnected)
	{
		if (ImGui::CollapsingHeader("Server data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// IP address
			static char ipBuffer[64] = "127.0.0.1";
			ImGui::InputText("IP", ipBuffer, sizeof(ipBuffer));

			// Port
			static int port = 3306;
			ImGui::InputInt("Port", &port);

			// Connect button
			ImGui::InputText("Login name", (char*)senderBuf.c_str(), sizeof(senderBuf));

			if (ImGui::Button("Connect"))
			{
				if (state == ClientState::Disconnected)
				{
					state = ClientState::Connecting;
				}
			}
		}
	}
	else if (state == ClientState::Connected)
	{
		// Disconnect button
		if (ImGui::Button("Disconnect"))
		{
			if (state == ClientState::Connected)
			{
				state = ClientState::Disconnecting;
			}
		}

		if (messengerState == MessengerState::ComposingMessage)
		{
			ImGui::InputText("Receiver", receiverBuf, sizeof(receiverBuf));
			ImGui::InputTextMultiline("Message", messageBuf, sizeof(messageBuf));
			if (ImGui::Button("Send"))
			{
				messengerState = MessengerState::SendingMessage;
			}
			if (ImGui::Button("Discard"))
			{
				messengerState = MessengerState::ShowingMessages;
			}
		}
		else if (messengerState == MessengerState::ShowingMessages)
		{
			if (ImGui::Button("Compose message"))
			{
				messengerState = MessengerState::ComposingMessage;
			}

			if (ImGui::Button("Refresh inbox"))
			{
				messengerState = MessengerState::RequestingMessages;
			}

			ImGui::Text("Inbox:");

			if (messages.empty()) {
				ImGui::Text(" - Your inbox is empty.");
			}

			/*int i = 0;
			for (auto &message : messages)
			{
				ImGui::PushID(i++);
				if (ImGui::TreeNode(&message, "%s - %s", message.senderUsername.c_str(), message.subject.c_str()))
				{
					ImGui::TextWrapped("%s", message.body.c_str());
					ImGui::TreePop();
				}
				ImGui::PopID();
			}*/
		}
	}

	ImGui::End();
}




// Low-level networking stuff...

void ModuleClient::connectToServer()
{
	// Create socket
	connSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket()");
	}

	// Connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(App->modServer->port);
	inet_pton(AF_INET, App->modServer->serverIP, &serverAddr.sin_addr);
	int res = connect(connSocket, (const sockaddr*)&serverAddr, sizeof(serverAddr));
	if (res == SOCKET_ERROR)
	{/*
		printWSError("connect()");
		LOG("Could not connect to the server %s:%d", App->modServer->serverIP, App->modServer->port);
		state = ClientState::Disconnecting;*/
		
	}
	else
	{
		state = ClientState::Connected;
		LOG("Server connected to %s:%d", App->modServer->serverIP, App->modServer->port);

		App->modServer->database()->Connect();
		messengerState = MessengerState::SendingLogin;
	}

	// Set non-blocking socket
	u_long nonBlocking = 1;
	res = ioctlsocket(connSocket, FIONBIO, &nonBlocking);
	if (res == SOCKET_ERROR) {
		printWSError("ioctlsocket() non-blocking");
		LOG("Could not set the socket in non-blocking mode.", App->modServer->serverIP, App->modServer->port);
		state = ClientState::Disconnecting;
	}

}

void ModuleClient::disconnectFromServer()
{
	closesocket(connSocket);
	recvBuffer.clear();
	recvPacketHead = 0;
	recvByteHead = 0;
	sendBuffer.clear();
	sendHead = 0;
	state = ClientState::Disconnected;
}

void ModuleClient::handleIncomingData()
{
	if (recvBuffer.size() - recvByteHead < RECV_CHUNK_SIZE) {
		recvBuffer.resize(recvByteHead + RECV_CHUNK_SIZE);
	}

	int res = recv(connSocket, (char*)&recvBuffer[recvByteHead], RECV_CHUNK_SIZE, 0);
	if (res == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// Do nothing
		}
		else
		{
			printWSError("recv() - socket disconnected forcily");
			state = ClientState::Disconnecting;
		}
	}
	else
	{
		if (res == 0)
		{
			state = ClientState::Disconnecting;
			LOG("Disconnection from server");
			return;
		}

		recvByteHead += res;
		while (recvByteHead - recvPacketHead > HEADER_SIZE)
		{
			const size_t recvWindow = recvByteHead - recvPacketHead;
			const uint32_t packetSize = *(uint32_t*)&recvBuffer[recvPacketHead];
			if (recvWindow >= packetSize)
			{
				InputMemoryStream stream(packetSize - HEADER_SIZE);
				//std::copy(&recvBuffer[recvPacketHead + HEADER_SIZE], &recvBuffer[recvPacketHead + packetSize], (uint8_t*)stream.GetBufferPtr());
				memcpy(stream.GetBufferPtr(), &recvBuffer[recvPacketHead + HEADER_SIZE], packetSize - HEADER_SIZE);
				onPacketReceived(stream);
				recvPacketHead += packetSize;
			}
		}

		if (recvPacketHead >= recvByteHead)
		{
			recvPacketHead = 0;
			recvByteHead = 0;
		}
	}
}

void ModuleClient::handleOutgoingData()
{
	if (sendHead < sendBuffer.size())
	{
		int res = send(connSocket, (const char *)&sendBuffer[sendHead], (int)sendBuffer.size(), 0);
		if (res == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// Do nothing
			}
			else
			{
				printWSError("send()");
				state = ClientState::Disconnecting;
			}
		}
		else
		{
			sendHead += res;
		}

		if (sendHead >= sendBuffer.size())
		{
			sendHead = 0;
			sendBuffer.clear();
		}
	}
}