#include "Application.h"
#include "Log.h"

Application * App = nullptr;

enum class MainState
{
	Create,
	Start,
	Loop,
	CleanUp,
	Fail,
	Exit
};

/*
Client::SendPacketLogin(); 
Server::OnPacketRecieved(); 
Server::OnPacketRecievedLogin(); 

Client::SendPacketQueryMessages(); 
Server::OnPacketReceivedAllMessages(); 
Server::SendPacket(); 
Client::OnPacketReceivedQueryAllMessagesRespond(); 

Client::SendPacketSendMessage(); 
Server::OnPacketReceivedSendMessage(); 

Extra funcionality: 
	- Password Login.
	- Delete Message. 
	- Search. 
	- Date and Time. 


*/

int main(int argc, char **argv)
{
	int result = EXIT_FAILURE;

	MainState state = MainState::Create;

	while (state != MainState::Exit)
	{
		switch (state)
		{
		case MainState::Create:
			App = new Application();
			if (App != nullptr) {
				state = MainState::Start;
			} else {
				LOG("Create failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Start:
			if (App->start()) {
				state = MainState::Loop;
			} else {
				LOG("Start failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Loop:
			if (App->update()) {
				if (App->wantsToExit()) {
					state = MainState::CleanUp;
				}
			} else {
				LOG("Loop failed");
				state = MainState::Fail;
			}
			break;

		case MainState::CleanUp:
			if (App->cleanUp()) {
				delete App;
				App = nullptr;
				state = MainState::Exit;
				result = EXIT_SUCCESS;
			} else {
				LOG("CleanUp failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Fail:
			LOG("Application failed :-(");
			break;

		case MainState::Exit:
			break;
		default:

			break;
		}
	}

	return result;
}
