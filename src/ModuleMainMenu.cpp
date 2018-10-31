#include "ModuleMainMenu.h"
#include "ModuleClient.h"
#include "ModuleServer.h"
#include "Application.h"
#include "Log.h"
#include "imgui/imgui.h"

bool ModuleMainMenu::start()
{
	start_time = std::clock();
	return true;
}

bool ModuleMainMenu::update()
{
	ImGui::Begin("Main menu");

	if (ImGui::Button("Connection configuration"))
		App->modServer->draw_config = !App->modServer->draw_config;

	clock_t current_time = clock() - start_time;

	switch (App->modClient->state)
	{
	case ClientState::Connecting:
		if (current_time % 1000 < 330)
			ImGui::Text("Connecting to server.");
		else if (current_time % 1000 > 330 && current_time % 1000 < 660)
			ImGui::Text("Connecting to server..");
		else
			ImGui::Text("Connecting to server...");

		if (current_time / CLOCKS_PER_SEC > 5 && !hosting_server)
		{
			hosting_server = true;
			App->modServer->setActive(true);
			App->modServer->state = ServerState::Starting;
		}

		if (current_time / CLOCKS_PER_SEC > 10)
			ImGui::Text("Connection  time lasting too long");

		if (current_time / CLOCKS_PER_SEC > 20)
		{
			App->modServer->cleanUp();
			App->modClient->state = ClientState::Disconnected;
			App->modClient->setActive(false);
			App->modServer->setActive(false);
		}

		break;

	case ClientState::Disconnected:
		ImGui::Text("Connection took too long. \nMake sure server configuration is correct");

		if (ImGui::Button("Retry"))
		{
			App->modServer->state = ServerState::Starting;
			App->modClient->state = ClientState::Connecting;
			App->modClient->setActive(true);
			App->modServer->setActive(true);
			start_time = std::clock();
		}

		break;
	}


	ImGui::End();

	return true;
}
