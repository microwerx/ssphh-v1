#include "stdafx.h"
#include <ssphh.hpp>

namespace SSPHH
{
	void SSPHH_Application::imguiShowTestWindow()
	{
		if (!Interface.tools.showTestsWindow)
			return;

		imguiWinX += imguiWinW + 64.0f;
		ImGui::SetNextWindowContentWidth(imguiWinW);
		ImGui::SetNextWindowPos(ImVec2(imguiWinX, 64));

		ImGui::Begin("TestWindow");
		ImGui::PushID("TestWindow");
		
		ImGui::Checkbox("Tests Window", &Interface.tools.showTestsWindow);

		ImGui::Separator();
		if (ImGui::Button("Test SPHL JSON Read/Save")) {
			// do tests
		}
		if (ImGui::Button("Test SPHL PPM Read/Save")) {
			// do tests
		}
		if (ImGui::Button("Test SPHL EXR Read/Save")) {
			// do tests
		}

		ImGui::Separator();
		ImGui::Text("Save SPHL JSON: %d", Interface.tests.bSaveSphlJSON);
		ImGui::Text("Read SPHL JSON: %d", Interface.tests.bReadSphlJSON);
		ImGui::Text("Save SPHL PPM:  %d", Interface.tests.bSaveSphlPPM);
		ImGui::Text("Read SPHL PPM:  %d", Interface.tests.bReadSphlPPM);
		ImGui::Text("Save SPHL EXR:  %d", Interface.tests.bSaveSphlEXR);
		ImGui::Text("Read SPHL EXR:  %d", Interface.tests.bReadSphlEXR);

		ImGui::PopID();
		ImGui::End();
	}
}