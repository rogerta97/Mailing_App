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
				CLIENTLOG("Create failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Start:
			if (App->start()) {
				state = MainState::Loop;
			} else {
				CLIENTLOG("Start failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Loop:
			if (App->update()) {
				if (App->wantsToExit()) {
					state = MainState::CleanUp;
				}
			} else {
				CLIENTLOG("Loop failed");
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
				CLIENTLOG("CleanUp failed");
				state = MainState::Fail;
			}
			break;

		case MainState::Fail:
			CLIENTLOG("Application failed :-(");
			break;

		case MainState::Exit:
			break;
		default:

			break;
		}
	}

	return result;
}
