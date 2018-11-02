#pragma once

#include "Module.h"
#include "database\DatabaseTypes.h"
#include <ctime>
#include <string>
#include <thread>

class ModuleMainMenu : public Module
{
public:

	bool start() override;

	bool update() override;

public:

	bool writing = false;

	std::thread connected_thread;
	std::thread writing_thread;
	std::thread getusers_thread;
	std::thread getmessages_thread;
	User selected_user;
private:
	
	bool logged = false;
	bool hosting_server = false;
	clock_t start_time;
	std::string warning_message;
};