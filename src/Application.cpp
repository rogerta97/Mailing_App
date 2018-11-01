#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleMainMenu.h"
#include "ModuleClient.h"
#include "ModuleServer.h"
#include "ModuleLogView.h"
#include "SocketUtils.h"

#define ADD_MODULE(ModuleClass, moduleAttribute) \
	moduleAttribute = new ModuleClass(); \
	modules.push_back(moduleAttribute);

static Application *g_Instance = nullptr;

Application::Application()
{
	// Create modules
	ADD_MODULE(ModuleWindow, modWindow);
	ADD_MODULE(ModuleMainMenu, modMainMenu);
	ADD_MODULE(ModuleClient, modClient);
	ADD_MODULE(ModuleServer, modServer);
	ADD_MODULE(ModuleLogView, modLogView);

	// Set active modules
	modWindow->setActive(true);
	modMainMenu->setActive(true);
	modLogView->setActive(true);
	modClient->setActive(true);
}


Application::~Application()
{
	// Destroy modules
	for (auto module : modules) {
		delete module;
	}
}


bool Application::start()
{
	initializeSocketsLibrary();

	for (auto module : modules) {
		if (module->start() == false) {
			return false;
		}
	}
	return true;
}

bool Application::update()
{
	if (doPreUpdate() == false) return false;
	
	if (doUpdate() == false) return false;

	if (doPostUpdate() == false) return false;

	return true;
}

bool Application::cleanUp()
{
	for (auto module : modules) {
		if (module->cleanUp() == false) {
			return false;
		}
	}

	cleanupSocketsLibrary();

	return true;
}

bool Application::doPreUpdate()
{
	for (auto module : modules)
	{
		if (module->isActive() == false) continue;
		
		if (module->preUpdate() == false) return false;
	}
	return true;
}

bool Application::doUpdate()
{
	for (auto module : modules)
	{
		if (module->isActive() == false) continue;

		if (module->update() == false) return false;
	}
	return true;
}

bool Application::doPostUpdate()
{
	for (auto module : modules)
	{
		if (module->isActive() == false) continue;

		if (module->postUpdate() == false) return false;
	}
	return true;
}


tm Application::getDateTime()
{
	tm date_time = tm();
	time_t rawtime = time(&rawtime);
	localtime_s(&date_time, (const time_t*)&rawtime);
	return date_time;
}

std::string Application::DateTimeToString(const tm& date_time)
{
	std::string ret;

	ret += (std::to_string(date_time.tm_year + 1900) + '/');
	ret += (std::to_string(date_time.tm_mon) + '/');

	if(date_time.tm_mday >= 10)
		ret += (std::to_string(date_time.tm_mday) + ' ');
	else
		ret += ('0' + std::to_string(date_time.tm_mday) + ' ');

	ret += (std::to_string(date_time.tm_hour) + ':');
	ret += (std::to_string(date_time.tm_min) + ':');
	ret += std::to_string(date_time.tm_sec);

	return ret;
}

std::tm Application::StringToDateTime(const std::string& string)
{
	std::tm ret;
	char* cursor = (char*)string.c_str();
	char* end_cursor = cursor + 4;

	ret.tm_year = strtoul(cursor, &end_cursor, 10);
	cursor = end_cursor + 1; end_cursor = cursor + 2;
	ret.tm_year = strtoul(cursor, &end_cursor, 10);
	cursor = end_cursor + 1; end_cursor = cursor + 2;
	ret.tm_year = strtoul(cursor, &end_cursor, 10);

	cursor = end_cursor + 1; end_cursor = cursor + 2;
	ret.tm_year = strtoul(cursor, &end_cursor, 10);
	cursor = end_cursor + 1; end_cursor = cursor + 2;
	ret.tm_year = strtoul(cursor, &end_cursor, 10);
	cursor = end_cursor + 1; end_cursor = cursor + 2;
	ret.tm_year = strtoul(cursor, &end_cursor, 10);

	return ret;
}
