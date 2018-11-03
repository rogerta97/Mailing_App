#pragma once

#include "IDatabaseGateway.h"
#include "DBConnection.h"

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

	std::vector<Message> getAllMessagesReceivedByUser(const std::string &username, const std::string &sender) override;
	std::vector<User> getAllUsers() override;
	User getUserData(const std::string &username) override;

	void sendConnectedPing(const std::string &username) override;
	void sendWritingPing(const std::string &username) override;


	void Connect() override { db = new DBConnection(bufMySqlHost, bufMySqlPort, bufMySqlDatabase, bufMySqlUsername, bufMySqlPassword);};
	void Disconnect() override { if(db) delete db; }
	void Reconnect() override { Disconnect(); Reconnect(); }

private:

	// Text buffers for ImGUI
	char bufMySqlHost[64] = "localhost";
	char bufMySqlPort[64] = "3306";
	char bufMySqlDatabase[64] = "mailing_app";
	char bufMySqlTable[64] = "messages";
	char bufMySqlUsername[64] = "rodrigo";
	char bufMySqlPassword[64] = "rodrigodpl";

	DBConnection* db = nullptr;
};

