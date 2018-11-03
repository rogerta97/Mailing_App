#pragma once

#include "IDatabaseGateway.h"
#include <vector>

class SimulatedDatabaseGateway :
	public IDatabaseGateway
{
public:

	// Constructor and destructor

	SimulatedDatabaseGateway();

	~SimulatedDatabaseGateway();


	// Virtual methods from IDatabaseGateway
	void insertMessage(const Message &message) override;
	void insertUser(const User &user) override;

	std::vector<Message> getAllMessagesReceivedByUser(const std::string &username, const std::string &sender) override;
	std::vector<User> getAllUsers() override;
	User getUserData(const std::string &username);

	void sendConnectedPing(const std::string &username) {};
	void sendWritingPing(const std::string &username) {};

	void Connect() {};
	void Disconnect() {};
	void Reconnect() {};

private:

	std::vector<Message> allMessages;
};
