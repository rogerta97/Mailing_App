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

	std::vector<Message> getAllMessagesReceivedByUser(const std::string &username) override;
	std::vector<User> getAllUsers() override;
	User getUserData(const std::string &username);

	void sendConnectedPing(const std::string &username);
	void sendWritingPing(const std::string &username);

	virtual void updateGUI() override;

	void Connect() { db = new DBConnection(bufMySqlHost, bufMySqlPort, bufMySqlDatabase, bufMySqlUsername, bufMySqlPassword);}
	void Disconnect() { if(db) delete db; }
	void Reconnect() { Disconnect(); Reconnect(); }

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

