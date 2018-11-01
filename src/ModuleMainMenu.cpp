#include "ModuleMainMenu.h"
#include "ModuleClient.h"
#include "ModuleServer.h"
#include "Application.h"
#include "Log.h"
#include "database\MySqlDatabaseGateway.h"
#include "imgui/imgui.h"

bool ModuleMainMenu::start()
{
	start_time = std::clock();
	return true;
}

bool ModuleMainMenu::update()
{
	ImGui::Begin("Main menu");

	if (!logged)
	{
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

		case ClientState::Connected:

			static char user_buffer[64];
			ImGui::InputText("Username", user_buffer, sizeof(user_buffer));

			static char password_buffer[64];
			ImGui::InputText("Password", password_buffer, sizeof(password_buffer), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);

			if (ImGui::Button("Login") && std::string(user_buffer).length() > 0 && std::string(password_buffer).length() > 0)
			{
				User user = App->modServer->database()->getUserData(user_buffer);
				if (user.username.length() == 0)
					warning_message = "Username not found.";
				else if (user.password != password_buffer)
					warning_message = "Incorrect password.";
				else {
					App->modClient->senderBuf = user_buffer;
					logged = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Sign in") && std::string(user_buffer).length() > 0 && std::string(password_buffer).length() > 0)
			{
				User user = App->modServer->database()->getUserData(user_buffer);
				if (user.username.length() == 0)
				{
					User new_user;
					new_user.username = user_buffer;
					new_user.password = password_buffer;
					new_user.last_connected = App->getDateTime();
					App->modServer->database()->insertUser(new_user);

					App->modClient->senderBuf = user_buffer;
					logged = true;
				}
				else
					warning_message = "An user with that username already exists.";
			}

			if (warning_message.length() != 0)
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), warning_message.c_str());

			break;
		}

		if (ImGui::Button("Connection configuration"))
			App->modServer->draw_config = !App->modServer->draw_config;
	}
	else
	{

	}


	ImGui::End();

	return true;
}
