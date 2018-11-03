#pragma once

#include "Module.h"
#include "SocketUtils.h"
#include "serialization/MemoryStream.h"
#include <list>

// Data members

enum class ServerState
{
	Off,
	Starting,
	Running,
	Stopping
};

// Client buffers
struct ClientStateInfo
{
	// Client socket
	SOCKET socket;

	// Recv buffer state
	size_t recvPacketHead = 0;
	size_t recvByteHead = 0;
	std::vector<uint8_t> recvBuffer;

	// Send buffer state
	size_t sendHead = 0;
	std::vector<uint8_t> sendBuffer;

	// Login
	std::string loginName;

	// bool should it be deleted?
	bool invalid = false;
};

class IDatabaseGateway;

class ModuleServer : public Module
{
public:

	// Constructor and destructor

	ModuleServer();

	~ModuleServer();


	// Virtual methods from parent class Module

	bool update() override;

	bool cleanUp() override;

	// Database

	IDatabaseGateway *database();

public:

	// Server state
	ServerState state = ServerState::Off;

	// Application port
	int port = 8000;

	// IP address of the server
	char serverIP[32] = "127.0.0.1";

	bool g_SimulateDatabaseConnection = false;

	// List with all connected clients
	std::list<ClientStateInfo> clients;

private:

	// Methods involving serialization / deserialization (contain TODOs)

	void onPacketReceived(SOCKET socket, const InputMemoryStream& stream);

	void onPacketReceivedLogin(SOCKET socket, const InputMemoryStream& stream);

	void onPacketReceivedQueryAllMessages(SOCKET socket, const InputMemoryStream& stream);

	void onPacketReceivedQueryAllUsers(SOCKET socket, const InputMemoryStream& stream);
	
	void onPacketReceivedSendMessage(SOCKET socket, const InputMemoryStream& stream);

	void onPacketReceivedConnectionPing(SOCKET socket, const InputMemoryStream& stream);

	void onPacketReceivedWritingPing(SOCKET socket, const InputMemoryStream& stream);

	void sendPacketQueryAllMessagesResponse(SOCKET socket, const std::string &username, const std::string &sender);

	void sendPacketQueryAllUsersResponse(SOCKET socket);

	void sendPacket(SOCKET socket, OutputMemoryStream& stream);

	// Low level networking stuff

	void startServer();

	void stopServer();

	void handleIncomingData();

	void handleOutgoingData();

	void handleIncomingDataFromClient(ClientStateInfo &info);

	void handleOutgoingDataToClient(ClientStateInfo &info);

	std::vector<SOCKET> getAllSockets() const;


	// Client management

	void createClientStateInfoForSocket(SOCKET s);

	ClientStateInfo & getClientStateInfoForSocket(SOCKET s);

	bool existsClientStateInfoForSocket(SOCKET s);

	void deleteInvalidSockets();



	// A gateway to database operations
	IDatabaseGateway *simulatedDatabaseGateway;
	IDatabaseGateway *mysqlDatabaseGateway;

	// Special socket to accept incoming client connections.
	SOCKET listenSocket;

	

};