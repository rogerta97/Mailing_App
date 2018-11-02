#pragma once

#include <string>
#include <ctime>

struct Message
{
	std::string senderUsername;
	std::string receiverUsername;
	std::string body;
	tm sent_time;
	bool is_received = false;
	bool is_read = false;
};


struct User
{
	std::string username;
	std::string password;
	tm last_connected;
	tm last_writing;
};
