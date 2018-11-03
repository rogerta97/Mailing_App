#include "ModuleMainMenu.h"
#include "ModuleClient.h"
#include "ModuleServer.h"
#include "Application.h"
#include "Log.h"
#include "database\MySqlDatabaseGateway.h"
#include "imgui/imgui.h"

#include <Windows.h>


void AuxiliarThread()
{
	while (true)
	{
		if (App->modMainMenu->iteration % 400 == 0)
		{
			App->modClient->sendPacketConnectedPing();
			App->modMainMenu->iteration++;
		}

		if ((App->modMainMenu->iteration + 100) % 400 == 0)
		{
			if (App->modMainMenu->writing)
			{
				App->modClient->sendPacketWritingPing();
				App->modMainMenu->iteration++;
			}
		}

		if ((App->modMainMenu->iteration + 200) % 400 == 0)
		{
			App->modClient->sendPacketUsersRequest();
			App->modMainMenu->iteration++;
		}

		if (App->modMainMenu->selected_user.username.size() > 0)
		{
			if ((App->modMainMenu->iteration + 300) % 400 == 0)
			{
				App->modClient->sendPacketQueryMessages(App->modMainMenu->selected_user.username.c_str());
				App->modMainMenu->iteration++;
			}
		}

	}
}

bool ModuleMainMenu::start()
{
	start_time = std::clock();
	return true;
}

bool ModuleMainMenu::update()
{
	ImGui::Begin("Main menu");

	clock_t current_time = clock() - start_time;

	if (!logged)
	{
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
				else 
					logged = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Sign in") && std::string(user_buffer).length() > 0 && std::string(password_buffer).length() > 0)
			{
				User user = App->modServer->database()->getUserData(user_buffer);
				if (user.username.length() == 0)
					logged = true;
				else
					warning_message = "An user with that username already exists.";
			}

			if (logged)
			{
				App->modClient->senderBuf = user_buffer;
				App->modClient->sendPacketLogin(user_buffer);
				aux_thread = std::thread(AuxiliarThread);
			}
			else if (warning_message.length() != 0)
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), warning_message.c_str());

			break;
		}

		if (ImGui::Button("Connection configuration"))
			App->modServer->draw_config = !App->modServer->draw_config;
	}
	else
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Online users:");
		tm current_time = App->getDateTime();
		current_time.tm_year += 1900;
		static int selected = -1;
		int count = 0;

		for (auto it = App->modClient->current_users.begin(); it != App->modClient->current_users.end(); it++)
		{
			if (App->modClient->senderBuf == (*it).username)
				continue;

			if (App->CompareDateTime(current_time, (*it).last_connected))
			{
				count++;
				if (ImGui::Selectable((*it).username.c_str(), selected == count))
				{
					selected = count;
					selected_user = *it;
				}
				if (App->CompareDateTime(current_time, (*it).last_writing))
				{
					ImGui::SameLine();
					ImGui::Text("Writing...");
				}
			}
		}

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Offline users:");
		for (auto it = App->modClient->current_users.begin(); it != App->modClient->current_users.end(); it++)
		{
			if (App->modClient->senderBuf == (*it).username)
				continue;

			if (!App->CompareDateTime(current_time, (*it).last_connected))
			{
				count++;
				if (ImGui::Selectable((*it).username.c_str(), selected == count))
				{
					selected = count;
					selected_user = *it;
				}
			}
		}

		if (selected_user.username.size() > 0)
			DrawChatWindow();
	}

	ImGui::End();

	iteration++;

	return true;
}


void ModuleMainMenu::DrawChatWindow()
{
	ImGui::Begin(selected_user.username.c_str());
	for (auto it = App->modClient->messages.begin(); it != App->modClient->messages.end(); it++)
	{
		if ((*it).senderUsername == App->modClient->senderBuf)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (App->modClient->senderBuf + ':').c_str());
		else if ((*it).senderUsername == selected_user.username)
			ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), (selected_user.username + ':').c_str());

		ImGui::SameLine();
		ImGui::Text((*it).body.c_str());
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), App->DateTimeToString((*it).sent_time, false).c_str());

		if ((*it).is_read)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "(read)");
		}

	}

	ImGui::InputTextMultiline("Message", messageBuf, sizeof(messageBuf));

	if (std::string(messageBuf).size() > 0)
	{
		writing = true;
		if (GetKeyState(VK_RETURN) & 0x8000 || ImGui::Button("Send"))
			App->modClient->sendPacketSendMessage(selected_user.username.c_str(), messageBuf);
	}
	else
		writing = false;

	ImGui::End();
}