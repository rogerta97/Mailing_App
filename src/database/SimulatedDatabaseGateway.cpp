#include "SimulatedDatabaseGateway.h"



SimulatedDatabaseGateway::SimulatedDatabaseGateway()
{
}


SimulatedDatabaseGateway::~SimulatedDatabaseGateway()
{
}

void SimulatedDatabaseGateway::insertMessage(const Message & message)
{
	allMessages.push_back(message);
}

std::vector<Message> SimulatedDatabaseGateway::getAllMessagesReceivedByUser(const std::string & username, const std::string &sender)
{
	std::vector<Message> messages;
	for (const auto & message : allMessages)
	{
		if (message.receiverUsername == username)
		{
			messages.push_back(message);
		}
	}
	return messages;
}



std::vector<User> SimulatedDatabaseGateway::getAllUsers()
{
	std::vector<User> users;

	return users;
}

User SimulatedDatabaseGateway::getUserData(const std::string & username)
{
	User user;
	return user;
}


void SimulatedDatabaseGateway::insertUser(const User & user)
{
}