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

bool Application::CompareDateTime(const tm& datetime_a, const tm& datetime_b, int margin_in_seconds)
{
	if (datetime_a.tm_year == datetime_b.tm_year && datetime_a.tm_mon == datetime_b.tm_mon && datetime_a.tm_mday == datetime_b.tm_mday && datetime_a.tm_hour == datetime_b.tm_hour)
	{
		if ((datetime_a.tm_min == datetime_b.tm_min && std::abs(datetime_a.tm_sec - datetime_b.tm_sec) < margin_in_seconds) ||
			(datetime_a.tm_min + 1 == datetime_b.tm_min && std::abs((datetime_a.tm_sec + 60) - datetime_b.tm_sec) < margin_in_seconds ||
				datetime_a.tm_min == datetime_b.tm_min + 1 && std::abs((datetime_a.tm_sec) - (datetime_b.tm_sec + 60)) < margin_in_seconds))
			return true;
	}
	return false;
}

std::string Application::DateTimeToString(const tm& date_time, bool add_1900)
{
	std::string ret;

	ret += (std::to_string(date_time.tm_year + (add_1900 ? 1900 : 0)) + '/');

	if (date_time.tm_mon >= 10) ret += (std::to_string(date_time.tm_mon) + '/');
	else						ret += ('0' + std::to_string(date_time.tm_mon) + '/');

	if(date_time.tm_mday >= 10) ret += (std::to_string(date_time.tm_mday) + ' ');
	else						ret += ('0' + std::to_string(date_time.tm_mday) + ' ');

	if (date_time.tm_hour >= 10) ret += (std::to_string(date_time.tm_hour) + ':');
	else						ret += ('0' + std::to_string(date_time.tm_hour) + ':');

	if (date_time.tm_min >= 10) ret += (std::to_string(date_time.tm_min) + ':');
	else						ret += ('0' + std::to_string(date_time.tm_min) + ':');

	if (date_time.tm_sec >= 10) ret += (std::to_string(date_time.tm_sec));
	else						ret += ('0' + std::to_string(date_time.tm_sec));

	return ret;
}

std::tm Application::StringToDateTime(const std::string& string)
{
	std::tm ret;
	if (string.size() > 0)
	{
		char* cursor = (char*)string.c_str();
		char* end_cursor = cursor + 4;

		ret.tm_year = strtoul(cursor, &end_cursor, 10);
		cursor = end_cursor + 1; end_cursor = cursor + 2;
		ret.tm_mon = strtoul(cursor, &end_cursor, 10);
		cursor = end_cursor + 1; end_cursor = cursor + 2;
		ret.tm_mday = strtoul(cursor, &end_cursor, 10);

		cursor = end_cursor + 1; end_cursor = cursor + 2;
		ret.tm_hour = strtoul(cursor, &end_cursor, 10);
		cursor = end_cursor + 1; end_cursor = cursor + 2;
		ret.tm_min = strtoul(cursor, &end_cursor, 10);
		cursor = end_cursor + 1; end_cursor = cursor + 2;
		ret.tm_sec = strtoul(cursor, &end_cursor, 10);
	}

	return ret;
}
