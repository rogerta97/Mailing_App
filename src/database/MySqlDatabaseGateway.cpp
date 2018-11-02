#include "MySqlDatabaseGateway.h"
#include "DBConnection.h"
#include "../imgui/imgui.h"
#include <cstdarg>

#include "../Application.h"


// You can use this function to create the SQL statements easily, works like the printf function
std::string stringFormat(const char *fmt, ...)
{
	// Find out the final string length
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(nullptr, 0, fmt, ap);
	va_end(ap);

	// Format the actual resulting string
	std::string resultString;
	resultString.resize(size + 1, '\0');
	va_start(ap, fmt);
	vsnprintf(&resultString[0], resultString.size(), fmt, ap);
	va_end(ap);

	return resultString;
}


MySqlDatabaseGateway::MySqlDatabaseGateway()
{
}


MySqlDatabaseGateway::~MySqlDatabaseGateway()
{
}

void MySqlDatabaseGateway::insertMessage(const Message & message)
{
	if (db)
	{
		if (db->isConnected())
		{
			DBResultSet res;

			// insert some messages
			db->sql(stringFormat("INSERT INTO messages (sender, receiver, body, sent_time, is_received, is_read) VALUES('%s', '%s', '%s', '%s', false, false)",
				message.senderUsername.c_str(), message.receiverUsername.c_str(), message.body.c_str()).c_str());
		}
		else
			Reconnect();
	}
	else
		Connect();
}

void MySqlDatabaseGateway::insertUser(const User & user)
{
	if (db)
	{
		if (db->isConnected())
		{
			DBResultSet res;

			std::string date_time = App->DateTimeToString(App->getDateTime());
			std::string writing_time = App->DateTimeToString(App->getDateTime());

			db->sql(stringFormat("INSERT INTO users VALUES('%s', '%s', '%s', '%s')",
				user.username.c_str(), user.password.c_str(), date_time.c_str(), writing_time.c_str()).c_str());
		}
		else
			Reconnect();
	}
	else
		Connect();
}

void MySqlDatabaseGateway::sendConnectedPing(const std::string &username)
{
	if (db)
	{
		if (db->isConnected())
		{
			DBResultSet res;

			std::string date_time = App->DateTimeToString(App->getDateTime());

			db->sql(stringFormat("update users set connectedTime = '%s' where username = '%s'",
				date_time.c_str(), username.c_str()).c_str());
		}
		else
			Reconnect();
	}
	else
		Connect();
}

void MySqlDatabaseGateway::sendWritingPing(const std::string &username)
{
	if (db)
	{
		if (db->isConnected())
		{
			DBResultSet res;

			std::string date_time = App->DateTimeToString(App->getDateTime());

			db->sql(stringFormat("update users set writingTime = '%s' where username = '%s'",
				date_time.c_str(), username.c_str()).c_str());
		}
		else
			Reconnect();
	}
	else
		Connect();
}

void MySqlDatabaseGateway::UpdateReadMessages(const std::string &sender, const std::string &receiver)
{
	if (db)
	{
		if (db->isConnected())
		{
			DBResultSet res;

			db->sql(stringFormat("update messages set is_read = true where (sender = '%s' and receiver = '%s')",
				sender.c_str(), receiver.c_str()).c_str());
		}
		else
			Reconnect();
	}
	else
		Connect();
}


std::vector<Message> MySqlDatabaseGateway::getAllMessagesReceivedByUser(const std::string & username, const std::string &sender)
{
	std::vector<Message> messages;

	if (db)
	{
		if (db->isConnected())
		{
			std::string sqlStatement;

			// consult all messages
			DBResultSet res = db->sql(stringFormat("select* from messages where(receiver = '%s' or sender = '%s')", username.c_str(), sender.c_str()).c_str());

			// fill the array of messages
			for (auto & messageRow : res.rows)
			{
				Message message;
				message.senderUsername = messageRow.columns[1];
				message.receiverUsername = messageRow.columns[2];
				message.body = messageRow.columns[3];
				message.sent_time = App->StringToDateTime(messageRow.columns[4]);
				if (message.receiverUsername == username)
				{
					message.is_received = true;
					db->sql(stringFormat("update messages set is_received = true where (id = '%s')", messageRow.columns[0].c_str()).c_str());
				}
				messages.push_back(message);
			}
		}
		else
			Reconnect();
	}
	else
		Connect();

	return messages;
}


std::vector<User> MySqlDatabaseGateway::getAllUsers()
{
	std::vector<User> users;

	if (db)
	{
		if (db->isConnected())
		{
			std::string sqlStatement;

			DBResultSet res = db->sql(stringFormat("select* from users").c_str());

			for (auto & userRow : res.rows)
			{
				User user;
				user.username = userRow.columns[0];
				user.password = userRow.columns[1];
				user.last_connected = App->StringToDateTime(userRow.columns[2]);
				user.last_writing = App->StringToDateTime(userRow.columns[3]);
				users.push_back(user);
			}
		}
		else
			Reconnect();
	}
	else
		Connect();


	return users;
}

User MySqlDatabaseGateway::getUserData(const std::string & username)
{
	User user;
	if (db)
	{
		if (db->isConnected())
		{
			std::string sqlStatement;

			DBResultSet res = db->sql(stringFormat("select* from users where(username = '%s')", username.c_str()).c_str());

			if (res.rows.size() != 0)
			{
				user.username = res.rows[0].columns[0];
				user.password = res.rows[0].columns[1];
				user.last_connected = App->StringToDateTime(res.rows[0].columns[2]);
				user.last_writing = App->StringToDateTime(res.rows[0].columns[3]);
			}
		}
		else
			Reconnect();
	}
	else
		Connect();

	return user;
}

void MySqlDatabaseGateway::updateGUI()
{
	ImGui::Separator();

	ImGui::Text("MySQL Server info");
	ImGui::InputText("Host", bufMySqlHost, sizeof(bufMySqlHost));
	ImGui::InputText("Port", bufMySqlPort, sizeof(bufMySqlPort));
	ImGui::InputText("Database", bufMySqlDatabase, sizeof(bufMySqlDatabase));
	ImGui::InputText("Username", bufMySqlUsername, sizeof(bufMySqlUsername));
	ImGui::InputText("Password", bufMySqlPassword, sizeof(bufMySqlUsername), ImGuiInputTextFlags_Password);
}
