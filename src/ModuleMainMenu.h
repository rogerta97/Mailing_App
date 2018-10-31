#pragma once

#include "Module.h"
#include <ctime>

class ModuleMainMenu : public Module
{
public:

	bool start() override;

	bool update() override;

private:
	 
	bool connected = false;
	bool hosting_server = false;
	clock_t start_time;
};