#include "SimulatedDatabaseGateway.h"
#include "../Application.h"


SimulatedDatabaseGateway::SimulatedDatabaseGateway()
{
	// some fake data

	for (int i = 0; i < 5; i++)
	{
		User client;
		client.username = "client" + std::to_string(i + 1);
		client.password = "12345";
		client.last_connected = client.last_writing = App->getDateTime();
		insertUser(client);
	}

	for (int i = 0; i < 6; i++)
	{
		Message msg;
		msg.body = "These are tests" + std::to_string(i);
		msg.is_read = false;
		msg.sent_time = App->getDateTime();
		msg.receiverUsername = i % 2 ? "client1" : "client2";
		msg.senderUsername = i % 2 ? "client2" : "client1";
		insertMessage(msg);
	}
}


SimulatedDatabaseGateway::~SimulatedDatabaseGateway()
{
	allMessages.clear();
	allUsers.clear();
}

void SimulatedDatabaseGateway::insertMessage(const Message & message)
{
	allMessages.push_back(message);
}

std::vector<Message> SimulatedDatabaseGateway::getAllMessagesReceivedByUser(const std::string & username, const std::string &sender)
{
	std::vector<Message> messages;

	for (int i = 0; i < allMessages.size(); i++)
	{
		if ((allMessages[i].receiverUsername == username && allMessages[i].senderUsername == sender) || (allMessages[i].receiverUsername == sender && allMessages[i].senderUsername == username))
		{
			messages.push_back(allMessages[i]);
			allMessages[i].is_read = true;
		}
	}

	return messages;
}


void SimulatedDatabaseGateway::sendConnectedPing(const std::string &username)
{
	for (int i = 0; i < allUsers.size(); i++)
	{
		if (allUsers[i].username == username)
		{
			allUsers[i].last_connected = App->getDateTime();
			return;
		}
	}
}
void SimulatedDatabaseGateway::sendWritingPing(const std::string &username)
{
	for (int i = 0; i < allUsers.size(); i++)
	{
		if (allUsers[i].username == username)
		{
			allUsers[i].last_writing = App->getDateTime();
			return;
		}
	}
}


std::vector<User> SimulatedDatabaseGateway::getAllUsers()
{
	return allUsers;
}

User SimulatedDatabaseGateway::getUserData(const std::string & username)
{
	for (int i = 0; i < allUsers.size(); i++)
	{
		if (allUsers[i].username == username)
			return allUsers[i];
	}

	User empty;
	return empty;
}


void SimulatedDatabaseGateway::insertUser(const User & user)
{
	allUsers.push_back(user);
}