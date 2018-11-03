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

	void DrawChatWindow();

public:

	bool writing = false;

	std::thread aux_thread;
	User selected_user;
	unsigned int iteration = 0;

private:
	
	bool logged = false;
	bool hosting_server = false;
	clock_t start_time;
	std::string warning_message;

	char messageBuf[200];// Buffer for the message
	unsigned int message_count = 0;
	bool scroll_to_bottom = true;
};