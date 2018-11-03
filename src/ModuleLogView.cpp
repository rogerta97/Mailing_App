#include "ModuleLogView.h"
#include "Log.h"
#include "imgui/imgui.h"

bool ModuleLogView::update()
{
	ImGui::Begin("Client log");

	for (int i = 0; i < ClientlogLineCount(); ++i)
	{
		const char *line = ClientlogLineAt(i);

		ImGui::TextWrapped("%s", line);
	}
	
	ImGui::End();

	if (ServerlogLineCount() > 0)
	{
		ImGui::Begin("Server log");

		for (int i = 0; i < ServerlogLineCount(); ++i)
		{
			const char *line = ServerlogLineAt(i);

			ImGui::TextWrapped("%s", line);
		}

		ImGui::End();
	}

	return true;
}
