#include "MySqlDatabaseGateway.h"
#include "DBConnection.h"
#include "../imgui/imgui.h"
#include <cstdarg>

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
	DBConnection db(bufMySqlHost, bufMySqlPort, bufMySqlDatabase, bufMySqlUsername, bufMySqlPassword);

	if (db.isConnected())
	{
		DBResultSet res;
		
		// insert some messages
		db.sql(stringFormat("INSERT INTO messages VALUES('%s', '%s', '%s', '%s')",
			message.senderUsername.c_str(), message.receiverUsername.c_str(), message.subject.c_str(), message.body.c_str()).c_str());
	}
}

std::vector<Message> MySqlDatabaseGateway::getAllMessagesReceivedByUser(const std::string & username)
{
	std::vector<Message> messages;

	DBConnection db(bufMySqlHost, bufMySqlPort, bufMySqlDatabase, bufMySqlUsername, bufMySqlPassword);

	if (db.isConnected())
	{
		std::string sqlStatement;
		
		// consult all messages
		DBResultSet res = db.sql(stringFormat("select* from messages where(sender = '%s')", username.c_str()).c_str());

		// fill the array of messages
		for (auto & messageRow : res.rows)
		{
			Message message;
			message.senderUsername = messageRow.columns[0];
			message.receiverUsername = messageRow.columns[1];
			message.subject = messageRow.columns[2];
			message.body = messageRow.columns[3];
			messages.push_back(message);
		}
	}


	return messages;
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
