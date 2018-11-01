#pragma once

#include "IDatabaseGateway.h"

class MySqlDatabaseGateway :
	public IDatabaseGateway
{
public:

	// Constructor and destructor

	MySqlDatabaseGateway();

	~MySqlDatabaseGateway();


	// Virtual methods from IDatabaseGateway

	void insertMessage(const Message &message) override;
	void insertUser(const User &user) override;

	std::vector<Message> getAllMessagesReceivedByUser(const std::string &username) override;
	std::vector<User> getAllUsers() override;
	User getUserData(const std::string &username);

	virtual void updateGUI() override;

private:

	// Text buffers for ImGUI
	char bufMySqlHost[64] = "localhost";
	char bufMySqlPort[64] = "3306";
	char bufMySqlDatabase[64] = "mailing_app";
	char bufMySqlTable[64] = "messages";
	char bufMySqlUsername[64] = "rodrigo";
	char bufMySqlPassword[64] = "rodrigodpl";
};

