#pragma once

#include <string>
#include <ctime>

struct Message
{
	std::string senderUsername;
	std::string receiverUsername;
	std::string subject;
	std::string body;
};


struct User
{
	std::string username;
	std::string password;
	tm last_connected;
	tm last_writing;
};
