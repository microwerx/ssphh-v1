#include "pch.hpp"
#include <ssphhapp.hpp>

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
		if (ImGui::Button("Test SPHLs")) {
			Interface.tests.bTestSPHLs = true;
			Interface.tests.bShowSPHLResults = true;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Show SPHL Results", &Interface.tests.bShowSPHLResults);
		if (Interface.tests.bShowSPHLResults) {
			ImGui::Text("Save SPHL JSON: %d", Interface.tests.saveSphlJSON);
			ImGui::Text("Read SPHL JSON: %d", Interface.tests.readSphlJSON);
			ImGui::Text("Save SPHL OBJ:  %d", Interface.tests.saveSphlOBJ);
			ImGui::Text("Save SPHL PPM:  %d", Interface.tests.saveSphlPPM);
			ImGui::Text("Read SPHL PPM:  %d", Interface.tests.readSphlPPM);
			ImGui::Text("Save SPHL EXR:  %d", Interface.tests.saveSphlEXR);
			ImGui::Text("Read SPHL EXR:  %d", Interface.tests.readSphlEXR);
		}
		ImGui::Separator();
		ImGui::Text("0 = not done");
		ImGui::Text("1 = requested");
		ImGui::Text("2 = finished");
		ImGui::Text("3 = warning");
		ImGui::Text("4 = error");

		ImGui::PopID();
		ImGui::End();
	}
}