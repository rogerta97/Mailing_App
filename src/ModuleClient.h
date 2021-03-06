#pragma once

#include "Module.h"
#include "SocketUtils.h"
#include "database/DatabaseTypes.h"
#include "serialization/MemoryStream.h"

// Client connection state
enum class ClientState
{
	Disconnected,
	WaitingConnection,
	Connecting,
	Connected,
	Disconnecting
};

class ModuleClient : public Module
{
public:

	// Virtual methods from parent class Module

	bool update() override;

	bool cleanUp() override;

	void sendPacketConnectedPing();

	void sendPacketWritingPing();

	void sendPacketUsersRequest();

	void sendPacketQueryMessages(const char *sender);

	void sendPacketLogin(const char *username);

	void sendPacketSendMessage(const char *receiver, const char *message);
public:

	// State of the client
	ClientState state = ClientState::Connecting;

	std::string senderBuf = "loginName";   // Buffer for the sender

	std::vector<User> current_users;

	// All messages in the client inbox
	std::vector<Message> messages;

private:

	// Methods involving serialization / deserialization (contain TODOs)

	void onPacketReceived(const InputMemoryStream &stream);

	void onPacketReceivedQueryAllMessagesResponse(const InputMemoryStream &stream);

	void onPacketReceivedAllUsersResponse(const InputMemoryStream &stream);

	void sendPacket(const OutputMemoryStream &stream);


	// Low-level networking stuff

	void connectToServer();

	void disconnectFromServer();

	void handleIncomingData();

	void handleOutgoingData();

	// Socket to connect to the server
	SOCKET connSocket;

	// Send and receive buffers (low-level stuff)

	// Recv buffer state
	size_t recvPacketHead = 0;
	size_t recvByteHead = 0;
	std::vector<uint8_t> recvBuffer;

	// Send buffer state
	size_t sendHead = 0;
	std::vector<uint8_t> sendBuffer;

};