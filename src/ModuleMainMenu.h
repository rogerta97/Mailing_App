#pragma once

#include "Module.h"
#include <ctime>
#include <string>

class ModuleMainMenu : public Module
{
public:

	bool start() override;

	bool update() override;

private:
	
	bool logged = false;
	bool hosting_server = false;
	clock_t start_time;
	std::string warning_message;
};