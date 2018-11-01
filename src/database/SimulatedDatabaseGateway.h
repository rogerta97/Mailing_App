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

	std::vector<Message> getAllMessagesReceivedByUser(const std::string &username) override;
	std::vector<User> getAllUsers() override;
	User getUserData(const std::string &username);

private:

	std::vector<Message> allMessages;
};
