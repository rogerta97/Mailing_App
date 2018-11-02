#pragma once

#include "DatabaseTypes.h"
#include <vector>

class IDatabaseGateway
{
public:

	// Constructor and destructor

	IDatabaseGateway() { }
	
	virtual ~IDatabaseGateway() { }


	// Virtual methods

	virtual void insertMessage(const Message &message) = 0;
	virtual void insertUser(const User &user) = 0;

	virtual std::vector<Message> getAllMessagesReceivedByUser(const std::string &username) = 0;
	virtual std::vector<User> getAllUsers() = 0;

	virtual User getUserData(const std::string &username) = 0;

	virtual void sendConnectedPing(const std::string &username) = 0;
	virtual void sendWritingPing(const std::string &username) = 0;

	virtual void updateGUI() { };

	virtual void Connect() {};
	virtual void Disconnect() {};
	virtual void Reconnect() {};
};

