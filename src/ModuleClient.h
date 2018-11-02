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

// Current screen of the messenger app
enum class MessengerState
{
	SendingLogin,
	RequestingMessages,
	ReceivingMessages,
	ShowingMessages,
	ComposingMessage,
	SendingMessage
};

void sendConnectedPingThread();
void sendWritingPingThread();

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
public:

	// State of the client
	ClientState state = ClientState::Connecting;

	std::string senderBuf = "loginName";   // Buffer for the sender
	clock_t connected_ping_timer = 0;
	clock_t writing_ping_timer = 0;
	clock_t user_request_timer = 0;
	clock_t message_request_timer = 0;

	// Current screen of the messenger application
	MessengerState messengerState = MessengerState::SendingLogin;

	std::vector<User> current_users;

private:

	// Methods involving serialization / deserialization (contain TODOs)

	void updateMessenger();

	void onPacketReceived(const InputMemoryStream &stream);

	void onPacketReceivedQueryAllMessagesResponse(const InputMemoryStream &stream);

	void onPacketReceivedAllUsersResponse(const InputMemoryStream &stream);

	void sendPacketLogin(const char *username);

	void sendPacketSendMessage(const char *receiver,  const char *message);

	void sendPacketMessagesRead(const char *sender);

	void sendPacket(const OutputMemoryStream &stream);

	
	// GUI

	void updateGUI();


	// Low-level networking stuff

	void connectToServer();

	void disconnectFromServer();

	void handleIncomingData();

	void handleOutgoingData();

	// Socket to connect to the server
	SOCKET connSocket;

	// All messages in the client inbox
	std::vector<Message> messages;

	// Composing Message buffers (for IMGUI)
	char receiverBuf[64]; // Buffer for the receiver
	char messageBuf[4096];// Buffer for the message


	// Send and receive buffers (low-level stuff)

	// Recv buffer state
	size_t recvPacketHead = 0;
	size_t recvByteHead = 0;
	std::vector<uint8_t> recvBuffer;

	// Send buffer state
	size_t sendHead = 0;
	std::vector<uint8_t> sendBuffer;

};