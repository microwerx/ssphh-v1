// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/KASL/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com

#include "stdafx.h"
#include <ssphh.hpp>
#include <fluxions_corona_scene.hpp>
#include <fluxions_ssphh.hpp>

using namespace Fluxions;
using namespace std;

static const int AllBands = -100;
static const int AllDegrees = -1;

static float x = 64.0f;
static float w = 384.0f;

#ifdef __APPLE__
#define __unix__
#endif

#ifdef __unix__
auto DeleteFile = unlink;
#endif

CoronaSceneFile coronaScene;

struct COLORS
{
	ImVec4 Red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 Green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 Blue = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	ImVec4 Yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	ImVec4 Cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 Magenta = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	ImVec4 Black = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 White = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
} Colors;

static const string DebugShaderChoices[21] = {
	" 0 - Default",
	" 1 - Kd - Material Diffuse Color",
	" 2 - Ks - Material Specular Color",
	" 3 - N - Material Normal Map",
	" 4 - V - View Vector",
	" 5 - R - Reflection Vector",
	" 6 - L - Light Vector",
	" 7 - H - Half Vector",
	" 8 - N.L - I/pi * N.L",
	" 9 - Mobile BRDF 1",
	"10 - Mobile BRDF 2 + c_e",
	"11 - BRDF Diffuse Only",
	"12 - BRDF Specular Only",
	"13 - BRDF Environment Only",
	"14 - BRDF in toto",
	"15 - BRDF in toto + c_e",
	"16 - BRDF in toto + c_e + c_a",
	"17 - Closest SPHL only + Sun",
	"18 - N.L + shadow",
	"19 - ",
	"20 - "};

#define NEWLINE_SEPARATOR \
	ImGui::Separator();   \
	ImGui::NewLine();
#define SEPARATOR_NEWLINE \
	ImGui::NewLine();     \
	ImGui::Separator();

void SSPHH_Application::RenderImGuiHUD()
{
	x = 64.0f;
	w = 256.0f;

	imguiShowMenuBar();
	imguiShowToolWindow();
	imguiShowMaterialEditor();
	imguiShowScenegraphEditor();
	imguiShowSphlEditor();
	imguiUfShowWindow();
	imguiShowSSPHHWindow();
	imguiShowRenderConfigWindow();

	//ImGui::ShowUserGuide();
}

void SSPHH_Application::imguiShowMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("(dummy menu)", NULL, false, false);
			if (ImGui::MenuItem("New"))
			{
			}
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
			}
			if (ImGui::BeginMenu("Open Recent"))
			{
				ImGui::MenuItem("fish_hat.c");
				ImGui::MenuItem("fish_hat.inl");
				ImGui::MenuItem("fish_hat.h");
				if (ImGui::BeginMenu("More.."))
				{
					ImGui::MenuItem("Hello");
					ImGui::MenuItem("Sailor");
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
			}
			if (ImGui::MenuItem("Save As.."))
			{
			}
			ImGui::Separator();
			if (ImGui::BeginMenu("Options"))
			{
				static bool enabled = true;
				ImGui::MenuItem("Enabled", "", &enabled);
				ImGui::BeginChild("child", ImVec2(0, 60), true);
				for (int i = 0; i < 10; i++)
					ImGui::Text("Scrolling Text %d", i);
				ImGui::EndChild();
				static float f = 0.5f;
				static int n = 0;
				ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
				ImGui::InputFloat("Input", &f, 0.1f);
				ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Colors"))
			{
				for (int i = 0; i < ImGuiCol_COUNT; i++)
					ImGui::MenuItem(ImGui::GetStyleColName((ImGuiCol)i));
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Disabled", false)) // Disabled
			{
				IM_ASSERT(0);
			}
			if (ImGui::MenuItem("Checked", NULL, true))
			{
			}
			if (ImGui::MenuItem("Quit", "Alt+F4"))
			{
				glutLeaveMainLoop();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
			}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false))
			{
			} // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X"))
			{
			}
			if (ImGui::MenuItem("Copy", "CTRL+C"))
			{
			}
			if (ImGui::MenuItem("Paste", "CTRL+V"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Scenegraph Editor"))
			{
				Interface.tools.showScenegraphEditor = true;
			}
			if (ImGui::MenuItem("Material Editor"))
			{
				Interface.tools.showMaterialEditor = true;
			}
			if (ImGui::MenuItem("SSPL Editor"))
			{
				Interface.tools.showSphlEditor = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void SSPHH_Application::imguiShowToolWindow()
{
	///////////////////////////////////////////////////////////
	// T O O L   W I N D O W //////////////////////////////////
	///////////////////////////////////////////////////////////

	//ImGui::SetNextWindowSize(ImVec2(320, 200));
	ImGui::SetNextWindowContentWidth(w);
	ImGui::SetNextWindowPos(ImVec2(x, 64));
	ImGui::Begin("Tool Window");
	ImGui::PushID("ToolWindow");
	if (ImGui::Button("Hide GUI"))
	{
		Interface.showImGui = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Quit"))
	{
		glutLeaveMainLoop();
	}
	extern double g_Fps;
	float time_ms = (float)(1000.0 / g_Fps);
	ImGui::SameLine();
	ImGui::Text("[ %3.1f fps/ %3.1f ms ]", g_Fps, time_ms);
	if (ImGui::SmallButton("Save Stats"))
	{
		imguiToolsSaveStats();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Reset Stats"))
	{
		imguiToolsResetStats();
	}

	if (ImGui::SmallButton("Screenshot"))
	{
		imguiToolsTakeScreenshot();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Shadows"))
	{
		Interface.captureShadows = true;
	}

	ImGui::Checkbox("Scenegraph Editor", &Interface.tools.showScenegraphEditor);
	ImGui::Checkbox("Material Editor", &Interface.tools.showMaterialEditor);
	ImGui::Checkbox("Unicornfish", &Interface.tools.showUnicornfishWindow);
	ImGui::Checkbox("SSPHH", &Interface.tools.showSSPHHWindow);
	ImGui::Checkbox("Render Config", &Interface.tools.showRenderConfigWindow);
	ImGui::SameLine();
	ImGui::Checkbox("Debug view", &defaultRenderConfig.enableDebugView);
	ImGui::Checkbox("Sphl Editor", &Interface.tools.showSphlEditor);
	ImGui::PushID(1234);
	ImGui::SameLine();
	if (ImGui::SmallButton("+"))
	{
		imguiSphlAdd();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-"))
	{
		imguiSphlDelete();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("l-"))
	{
		imguiSphlDecreaseDegrees();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("l+"))
	{
		imguiSphlIncreaseDegrees();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("?"))
	{
		imguiSphlRandomize();
	}
	if (ImGui::SmallButton("Corona->SPH"))
	{
		if (!ssg.ssphhLights.empty())
		{
			imguiSphlCoronaToSPH(ssg.ssphhLights.size() - 1);
		}
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("SPH->CubeMap"))
	{
		if (!ssg.ssphhLights.empty())
		{
			imguiSphlSPHtoCubeMap(ssg.ssphhLights.size() - 1);
		}
	}
	ImGui::PopID();

	ImGui::Separator();

	imguiCoronaControls();
	ImGui::DragFloat("Exposure", &ssg.environment.toneMapExposure, 0.1f, -12.0f, 12.0f, "%.1f");
	ImGui::DragFloat("Gamma", &ssg.environment.toneMapGamma, 0.1f, 1.0f, 6.0f, "%.1f");

	ImGui::Separator();

	ImGui::Text("%s", DebugShaderChoices[Interface.tools.shaderDebugChoice].c_str());
	ImGui::Text("Shader Debug Choice: %02d", Interface.tools.shaderDebugChoice);
	ImGui::SameLine();
	if (ImGui::SmallButton("+"))
	{
		Interface.tools.shaderDebugChoice++;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-"))
	{
		Interface.tools.shaderDebugChoice--;
	}
	Interface.tools.shaderDebugChoice = Fluxions::clamp<int>(Interface.tools.shaderDebugChoice, 0, 20);

	ImGui::DragInt("Debug Light", &Interface.tools.shaderDebugLight, 0.1f, -1, 16);
	ImGui::DragInt("Debug SPHL ", &Interface.tools.shaderDebugSphl, 0.1f, -1, 16);

	ImGui::SameLine();
	if (ImGui::SmallButton("Same"))
	{
		for (int i = 0; i < 4; i++)
		{
			Interface.tools.shaderDebugChoices[i] = Interface.tools.shaderDebugChoice;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		ImGui::PushID(i);
		ImGui::Text("Shader %d / % 2d", i, Interface.tools.shaderDebugChoices[i]);
		ImGui::SameLine();
		if (ImGui::SmallButton("+"))
		{
			Interface.tools.shaderDebugChoices[i]++;
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("-"))
		{
			Interface.tools.shaderDebugChoices[i]--;
		}
		Interface.tools.shaderDebugChoices[i] = Fluxions::clamp<int>(Interface.tools.shaderDebugChoices[i], 0, 20);
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Text("CAMERA POS: % 3.2f/% 3.2f/ % 3.2f", Interface.cameraPosition.x, Interface.cameraPosition.y, Interface.cameraPosition.z);
	ImGui::Text("CAMERA POS: % 3.2f/% 3.2f/ % 3.2f", defaultRenderConfig.cameraPosition.x, defaultRenderConfig.cameraPosition.y, defaultRenderConfig.cameraPosition.z);
	if (ImGui::SmallButton("LoadIdentity()"))
	{
		Interface.preCameraMatrix.LoadIdentity();
	}
	if (ImGui::SmallButton("+X"))
	{
		Interface.preCameraMatrix.Translate(1.0f, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-X"))
	{
		Interface.preCameraMatrix.Translate(-1.0f, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("+Y"))
	{
		Interface.preCameraMatrix.Translate(0.0f, 1.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-Y"))
	{
		Interface.preCameraMatrix.Translate(0.0f, -1.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("+Z"))
	{
		Interface.preCameraMatrix.Translate(0.0f, 0.0f, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-Z"))
	{
		Interface.preCameraMatrix.Translate(0.0f, 0.0f, -1.0f);
	}
	if (ImGui::SmallButton("Reset Orbit"))
	{
		Interface.cameraOrbit.reset();
	}
	ImGui::SameLine();
	ImGui::Checkbox("Enable Orbit", &Interface.enableOrbit);
	if (ImGui::SmallButton("-Az"))
	{
		Interface.cameraOrbit.x -= 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("+Az"))
	{
		Interface.cameraOrbit.x += 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-Alt"))
	{
		Interface.cameraOrbit.y -= 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("+Alt"))
	{
		Interface.cameraOrbit.y += 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-R"))
	{
		Interface.cameraOrbit.z -= 1.0f;
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("+R"))
	{
		Interface.cameraOrbit.z += 1.0f;
	}

	ImGui::TextColored(Colors.Yellow, "Sun: ");
	ImGui::TextColored(Colors.Yellow, "%0.4fi %0.4fj %0.4fk", ssg.environment.curSunDirTo.x, ssg.environment.curSunDirTo.y, ssg.environment.curSunDirTo.z);
	ImGui::TextColored(Colors.Yellow, "Shadow map time: %.3fms", ssg.environment.sunShadowMapTime);
	ImGui::TextColored(Colors.Yellow, "Sun: %3.1f %3.1f %3.1f", ssg.environment.curSunDiskRadiance.r, ssg.environment.curSunDiskRadiance.g, ssg.environment.curSunDiskRadiance.b);
	ImGui::TextColored(Colors.Red, "Gnd: %3.1f %3.1f %3.1f", ssg.environment.curGroundRadiance.r, ssg.environment.curGroundRadiance.g, ssg.environment.curGroundRadiance.b);
	ImGui::Checkbox("Sun Cycle", &Interface.enableSunCycle);
	ImGui::SameLine();
	if (ImGui::SmallButton("+ 1hr"))
	{
		AdvanceSunClock(3600.0, true);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("- 1hr"))
	{
		AdvanceSunClock(-3600.0, true);
	}

	ImGui::Separator();

	ImGui::Text("Debug Viz %d", counter);
	ImGui::SameLine();
	if (ImGui::SmallButton("Disable Debug Viz"))
		counter = -1;
	if (ImGui::Button("Show SPHs"))
		if (counter != 0)
		{
			counter = 0;
		}
		else
		{
			counter = -1;
		}
	if (ImGui::Button("Show Sun Shadow Map"))
		if (counter != 1)
			counter = 1;
		else
		{
			counter = -1;
		}
	if (ImGui::Button("Show Sphere Cube Map"))
		if (counter != 2)
			counter = 2;
		else
		{
			counter = -1;
		}
	if (ImGui::Button("Show Enviro Cube Map"))
		if (counter != 3)
			counter = 3;
		else
		{
			counter = -1;
		}

	ImGui::Separator();

	ImGui::Checkbox("GL Info", &Interface.tools.showGLInfo);
	if (Interface.tools.showGLInfo)
	{
		if (ImGui::Button("GL Extensions") || Interface.tools.gl_extensions.empty())
		{
			Interface.tools.gl_extensions.clear();
			Interface.tools.gl_extensions_cstr.clear();
			string extensions = (const char *)glGetString(GL_EXTENSIONS);
			regex my_regex("\\w+");
			sregex_iterator begin = sregex_iterator(extensions.begin(), extensions.end(), my_regex);
			sregex_iterator end = sregex_iterator();
			for (auto it = begin; it != end; it++)
			{
				smatch match = *it;
				if (match.str().size() < 3)
					continue;
				Interface.tools.gl_extensions.push_back(match.str());
			}

			sort(Interface.tools.gl_extensions.begin(), Interface.tools.gl_extensions.end());

			for (auto &s : Interface.tools.gl_extensions)
			{
				Interface.tools.gl_extensions_cstr.push_back(s.c_str());
			}
			Interface.tools.gl_version = (const char *)glGetString(GL_VERSION);
			Interface.tools.gl_renderer = (const char *)glGetString(GL_RENDERER);
		}
		ImGui::Text("GL_RENDERER: %s", Interface.tools.gl_renderer.c_str());
		ImGui::Text("GL_VERSION:  %s", Interface.tools.gl_version.c_str());
		ImGui::Text("GL_EXTENSIONS: %d", Interface.tools.gl_extensions.size());
		if (!Interface.tools.gl_extensions.empty())
		{
			//ImGui::SetNext
			ImGui::PushItemWidth(w - 16);
			ImGui::ListBox("##empty", &Interface.tools.gl_extensions_curitem, &Interface.tools.gl_extensions_cstr[0], (int)Interface.tools.gl_extensions.size(), 16);
			ImGui::PopItemWidth();
		}
	}

	ImGui::PopID();
	ImGui::End();
}

void SSPHH_Application::imguiShowRenderConfigWindow()
{
	if (!Interface.tools.showRenderConfigWindow)
	{
		return;
	}
	ImGui::SetNextWindowPos(ImVec2(x + w + 32, 64));
	ImGui::Begin("Render Configuration");
	ImGui::Checkbox("Debug view", &defaultRenderConfig.enableDebugView);
	ImGui::Checkbox("Depth Test", &defaultRenderConfig.enableDepthTest);
	ImGui::Checkbox("Blend", &defaultRenderConfig.enableBlend);
	ImGui::Checkbox("Cull Face", &defaultRenderConfig.enableCullFace);
	ImGui::Checkbox("sRGB Framebuffer", &defaultRenderConfig.enableSRGB);
	ImGui::Checkbox("Shadow color buffer", &defaultRenderConfig.enableZColorBuffer);
	ImGui::Separator();
	ImGui::Text("ZNEAR: % .3f / ZFAR % .3f", defaultRenderConfig.znear, defaultRenderConfig.zfar);
	ImGui::Separator();
	ImGui::Text("Sun Shadows");
	ImGui::Text("ZNEAR: % .3f / ZFAR % .3f", rectShadowRenderConfig.znear, rectShadowRenderConfig.zfar);
	ImGui::SliderInt("2^N", &Interface.renderconfig.sunShadowMapSizeChoice, 6, 12);
	ImGui::SameLine();
	ImGui::Text("= %d", 2 << Interface.renderconfig.sunShadowMapSizeChoice);
	Interface.renderconfig.sunShadowMapSize = 2 << Interface.renderconfig.sunShadowMapSizeChoice;
	ImGui::SliderFloat("Shadow Map Zoom", &ssg.environment.sunShadowMapZoom, 0.1f, 2.0f);
	Vector2f before = ssg.environment.sunShadowMapOffset;
	ImGui::SliderFloat2("Offset", ssg.environment.sunShadowMapOffset.ptr(), -20.0f, 20.0f);
	if (before != ssg.environment.sunShadowMapOffset)
	{
		ssg.environment.Update(ssg.GetBoundingBox());
	}
	ImGui::SameLine();
	if (ImGui::Button("0"))
	{
		ssg.environment.sunShadowMapZoom = 1.0;
		ssg.environment.sunShadowMapOffset.reset();
	}
	ImGui::SameLine();
	if (ImGui::Button("!"))
	{
		ssg.environment.Update(ssg.GetBoundingBox());
	}
	ImGui::Checkbox("Shadow Cull Face", &rectShadowRenderConfig.enableCullFace);
	static bool frontOrBack = true;
	ImGui::Checkbox("Shadow Cull Front/Back", &frontOrBack);
	if (frontOrBack)
		rectShadowRenderConfig.cullFaceMode = GL_FRONT;
	else
		rectShadowRenderConfig.cullFaceMode = GL_BACK;
	ImGui::SameLine();
	ImGui::Text("%s", frontOrBack ? "Front" : "Back");

	ImGui::Separator();

	for (int i = 0; i < sphls.size(); i++)
	{
		const GLuint *texid = sphls[i].lightProbeTexIds;
		ImGui::Text("sphls[%d] light probes %d -- %d -- %d", i, texid[0], texid[1], texid[2]);
	}

	ImGui::End();
}

void SSPHH_Application::imguiShowScenegraphEditor()
{
	///////////////////////////////////////////////////////////
	// S C E N E G R A P H   E D I T O R //////////////////////
	///////////////////////////////////////////////////////////

	if (Interface.tools.showScenegraphEditor)
	{
		x += w + 64.0f;
		ImGui::SetNextWindowContentWidth(w * 2);
		ImGui::SetNextWindowPos(ImVec2(x, 64));
		x += w;
		ImGui::Begin("Scenegraph");
		static int sceneNumber = 0;
		ImGui::Combo("Scene", &sceneNumber, "test_indoor_scene\0test_outside_scene\0test_mitsuba_scene");
		if (ImGui::Button("Reload Scene"))
		{
			sceneFilename = "resources/scenes/test_texture_scene/";
			switch (sceneNumber)
			{
			case 0:
				sceneFilename += "test_indoor_scene.scn";
				break;
			case 1:
				sceneFilename += "test_outdoor_scene.scn";
				break;
			case 2:
				sceneFilename += "test_mitsuba_scene.scn";
				break;
			default:
				sceneFilename += "simple_inside_scene.scn";
				break;
			}
			ReloadScenegraph();
		}
		ImGui::Text("Load time: %3.2f msec", Interface.lastScenegraphLoadTime);
		if (ImGui::Button("Reload Render Config"))
		{
			ReloadRenderConfigs();
		}
		ImGui::Text("Load time: %3.2f msec", Interface.lastRenderConfigLoadTime);
		BoundingBoxf bbox = ssg.GetBoundingBox();
		Vector3f sceneMin(trunc(bbox.minBounds.x - 0.5f), trunc(bbox.minBounds.x - 0.5f), trunc(bbox.minBounds.z - 0.5f));
		Vector3f sceneMax(trunc(bbox.maxBounds.x + 0.5f), trunc(bbox.maxBounds.y + 0.5f), trunc(bbox.maxBounds.z + 0.5f));
		Vector3f sceneSize(trunc(bbox.X() + 0.5f), trunc(bbox.Y() + 0.5f), trunc(bbox.Z() + 0.5f));
		ImGui::Text("scene dimensions (%.1f %.1f %.1f)/(%.1f, %.1f, %.1f) [%.1f, %.1f, %.1f]",
					sceneMin.x, sceneMin.y, sceneMin.z,
					sceneMax.x, sceneMax.y, sceneMax.z,
					sceneSize.x, sceneSize.y, sceneSize.z);

		ImGui::Checkbox("Environment", &Interface.ssg.showEnvironment);
		if (Interface.ssg.showEnvironment)
		{
			ImGui::Checkbox("Environment Details", &Interface.ssg.showEnvironmentDetails);
			if (Interface.ssg.showEnvironmentDetails)
			{
				NEWLINE_SEPARATOR
				ImGui::Text("sunColorMap      Unit/Id/SamplerId: %2d/%2d/%2d", ssg.environment.sunColorMapUnit, ssg.environment.sunColorMapId, ssg.environment.sunColorMapSamplerId);
				ImGui::Text("sunDepthMap      Unit/Id/SamplerId: %2d/%2d/%2d", ssg.environment.sunDepthMapUnit, ssg.environment.sunDepthMapId, ssg.environment.sunDepthMapSamplerId);
				ImGui::Text("enviroColorMap   Unit/Id/SamplerId: %2d/%2d/%2d", ssg.environment.enviroColorMapUnit, ssg.environment.enviroColorMapId, ssg.environment.enviroColorMapSamplerId);
				ImGui::Text("enviroDepthMap   Unit/Id/SamplerId: %2d/%2d/%2d", ssg.environment.enviroDepthMapUnit, ssg.environment.enviroDepthMapId, ssg.environment.enviroDepthMapSamplerId);
				ImGui::Text("pbskyColorMap    Unit/Id/SamplerId: %2d/%2d/%2d", ssg.environment.pbskyColorMapUnit, ssg.environment.pbskyColorMapId, ssg.environment.pbskyColorMapSamplerId);
				ImGui::Text("shadow map: znear: %.1f, zfar: %0.1f, origin(%.1f, %.1f, %.1f), target(%.1f, %.1f, %.1f), up(%.2f, %.2f, %.2f)",
							ssg.environment.sunShadowMapNearZ,
							ssg.environment.sunShadowMapFarZ,
							ssg.environment.sunShadowMapOrigin.x, ssg.environment.sunShadowMapOrigin.y, ssg.environment.sunShadowMapOrigin.z,
							ssg.environment.sunShadowMapTarget.x, ssg.environment.sunShadowMapTarget.y, ssg.environment.sunShadowMapTarget.z,
							ssg.environment.sunShadowMapUp.x, ssg.environment.sunShadowMapUp.y, ssg.environment.sunShadowMapUp.z);
				SEPARATOR_NEWLINE
			}

			ImGui::TextColored(Colors.Red, "Camera: ");
			ImGui::DragFloat("FOV: ", &Interface.ssg.cameraFOV);
			Matrix4f m = ssg.camera.viewMatrix;
			ImGui::Text(
				"viewMatrix: % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f",
				m.m11, m.m12, m.m13, m.m14,
				m.m21, m.m22, m.m23, m.m24,
				m.m31, m.m32, m.m33, m.m34,
				m.m41, m.m42, m.m43, m.m44);
			m = Interface.preCameraMatrix;
			ImGui::Text(
				"altMatrix:  % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f\n"
				"            % 3.2f % 3.2f % 3.2f % 3.2f",
				m.m11, m.m12, m.m13, m.m14,
				m.m21, m.m22, m.m23, m.m24,
				m.m31, m.m32, m.m33, m.m34,
				m.m41, m.m42, m.m43, m.m44);
			ImGui::Text("CAMERA POS: % 3.2f/% 3.2f/ % 3.2f", Interface.cameraPosition.x, Interface.cameraPosition.y, Interface.cameraPosition.z);
			if (ImGui::SmallButton("LoadIdentity()"))
			{
				Interface.preCameraMatrix.LoadIdentity();
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+X"))
			{
				Interface.preCameraMatrix.Translate(1.0f, 0.0f, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-X"))
			{
				Interface.preCameraMatrix.Translate(-1.0f, 0.0f, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+Y"))
			{
				Interface.preCameraMatrix.Translate(0.0f, 1.0f, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-Y"))
			{
				Interface.preCameraMatrix.Translate(0.0f, -1.0f, 0.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+Z"))
			{
				Interface.preCameraMatrix.Translate(0.0f, 0.0f, 1.0f);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-Z"))
			{
				Interface.preCameraMatrix.Translate(0.0f, 0.0f, -1.0f);
			}
			ImGui::Text("Orbit");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset Orbit"))
			{
				Interface.cameraOrbit.reset();
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-Az"))
			{
				Interface.cameraOrbit.x -= 5.0f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+Az"))
			{
				Interface.cameraOrbit.x += 5.0f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-Alt"))
			{
				Interface.cameraOrbit.y -= 5.0f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+Alt"))
			{
				Interface.cameraOrbit.y += 5.0f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-R"))
			{
				Interface.cameraOrbit.z -= 1.0f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("+R"))
			{
				Interface.cameraOrbit.z += 1.0f;
			}

			ImGui::Text("WxH: %4.fx%4.f / FOV: %3.2f", ssg.camera.imageWidth, ssg.camera.imageHeight, ssg.camera.fov);
			ImGui::Text("ZNEAR: %1.4f / ZFAR: %4.1f", ssg.camera.imageNearZ, ssg.camera.imageFarZ);
			NEWLINE_SEPARATOR

			ImGui::TextColored(Colors.Yellow, "Sun: ");
			ImGui::SameLine();
			if (ImGui::SmallButton("+ 1hr"))
			{
				AdvanceSunClock(3600.0, true);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("- 1hr"))
			{
				AdvanceSunClock(-3600.0, true);
			}

			double turbidity = ssg.environment.pbsky.GetTurbidity();
			double lastTurbidity = turbidity;
			ImGui::Text("Turbidity: % 2.1f", turbidity);
			ImGui::SameLine();
			if (ImGui::SmallButton("+"))
			{
				turbidity += 1.0;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("-"))
			{
				turbidity -= 1.0;
			}
			turbidity = Fluxions::clamp<double>(turbidity, 1.0, 10.0);
			ssg.environment.pbsky.SetTurbidity((float)turbidity);
			if (turbidity != lastTurbidity)
			{
				ssg.environment.pbsky.ComputeSunFromLocale();
				ssg.environment.ComputePBSky();
			}

			ImGui::Text("DATE: %02d/%02d/%04d %02d:%02d:%02d%.3f -- LST : %2.3f",
						ssg.environment.pbsky.getMonth(),
						ssg.environment.pbsky.getDay(),
						ssg.environment.pbsky.getYear(),
						ssg.environment.pbsky.getHour(),
						ssg.environment.pbsky.getMin(),
						ssg.environment.pbsky.getSec(),
						ssg.environment.pbsky.getSecFract(),
						ssg.environment.pbsky.getLST());
			ImGui::Text("LAT: % 2.2f LONG: % 3.2f",
						ssg.environment.pbsky.GetLatitude(),
						ssg.environment.pbsky.GetLongitude());
			NEWLINE_SEPARATOR
		}

		ImGui::Checkbox("Geometry", &Interface.ssg.showGeometry);
		if (Interface.ssg.showGeometry)
		{
			NEWLINE_SEPARATOR
			if (Interface.ssg.geometryCollapsed.size() != ssg.geometry.size())
			{
				Interface.ssg.geometryCollapsed.resize(ssg.geometry.size());
			}

			int i = 0;
			for (auto &g : ssg.geometry)
			{
				ostringstream label;
				label << g.second.objectName << "(" << i << ")";
				ImGui::Checkbox(label.str().c_str(), &Interface.ssg.geometryCollapsed[i].second);
				if (Interface.ssg.geometryCollapsed[i].second)
				{
					ImGui::Text("name: %s", g.second.objectName.c_str());
					ImGui::Text("filename: %s", g.second.fpi.fname.c_str());
					ImGui::Text("mtllib: ", g.second.mtllibName.c_str());
					BoundingBoxf bbox = g.second.bbox;
					Vector3f sceneMin = bbox.minBounds;
					Vector3f sceneMax = bbox.maxBounds;
					Vector3f sceneSize = bbox.Size();
					ImGui::Text("dimensions (%.1f %.1f %.1f)/(%.1f, %.1f, %.1f) [%.1f, %.1f, %.1f]",
								sceneMin.x, sceneMin.y, sceneMin.z,
								sceneMax.x, sceneMax.y, sceneMax.z,
								sceneSize.x, sceneSize.y, sceneSize.z);
					//ImGui::Text("dimensions: %.2f/%.2f/%.2f", g.second.bbox.MaxX(), g.second.bbox.MaxY(), g.second.bbox.MaxZ());

					imguiMatrix4fEditControl(i, g.second.transform);
				}
				i++;
			}
			SEPARATOR_NEWLINE
		}

		ImGui::Checkbox("Point Lights", &Interface.ssg.showPointLights);
		if (Interface.ssg.showPointLights)
		{
			int i = 0;
			for (auto &spl : ssg.pointLights)
			{
				ostringstream ostr;
				ImGui::Text("Name(%d): %s", i, spl.name.c_str());
				ImGui::DragFloat("E0", &spl.E0, 0.1f, 0.0f, 5.0f);
				ImGui::DragFloat("Falloff Radius", &spl.falloffRadius, 1.0f, 0.0f, 1000.0f);
				ImGui::DragFloat3("Position", spl.position.ptr(), 0.1f, -10.0f, 10.0f);
				i++;
			}
		}

		ImGui::End();
	}
}

void SSPHH_Application::imguiMatrix4fEditControl(int id, Matrix4f &m)
{
	NEWLINE_SEPARATOR
	ImGui::Text("Rotate: ");
	if (ImGui::Button("LoadIdentity()"))
	{
		m.LoadIdentity();
	}
	ImGui::Text("X: ");
	ImGui::SameLine();
	if (ImGui::Button("-5deg X"))
	{
		m.Rotate(-5.0f, 1.0f, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("-1deg X"))
	{
		m.Rotate(-1.0f, 1.0f, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+1deg X"))
	{
		m.Rotate(1.0f, 1.0f, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+5deg X"))
	{
		m.Rotate(5.0f, 1.0f, 0.0f, 0.0f);
	}
	ImGui::Text("Y: ");
	ImGui::SameLine();
	if (ImGui::Button("-5deg Y"))
	{
		m.Rotate(-5.0f, 0.0f, 1.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("-1deg Y"))
	{
		m.Rotate(-1.0f, 0.0f, 1.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+1deg Y"))
	{
		m.Rotate(1.0f, 0.0f, 1.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+5deg Y"))
	{
		m.Rotate(5.0f, 0.0f, 1.0f, 0.0f);
	}
	ImGui::Text("Z: ");
	ImGui::SameLine();
	if (ImGui::Button("-5deg Z"))
	{
		m.Rotate(-5.0f, 0.0f, 0.0f, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("-1deg Z"))
	{
		m.Rotate(-1.0f, 0.0f, 0.0f, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+1deg Z"))
	{
		m.Rotate(1.0f, 0.0f, 0.0f, 1.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("+5deg Z"))
	{
		m.Rotate(5.0f, 0.0f, 0.0f, 1.0f);
	}

	ImGui::Text("Translate: ");
	ImGui::SameLine();
	ImGui::DragFloat4("", m.ptr() + 12, 0.1f);
	ImGui::Text(
		"transform: % 3.2f % 3.2f % 3.2f % 3.2f\n"
		"           % 3.2f % 3.2f % 3.2f % 3.2f\n"
		"           % 3.2f % 3.2f % 3.2f % 3.2f\n"
		"           % 3.2f % 3.2f % 3.2f % 3.2f",
		m.m11, m.m12, m.m13, m.m14,
		m.m21, m.m22, m.m23, m.m24,
		m.m31, m.m32, m.m33, m.m34,
		m.m41, m.m42, m.m43, m.m44);
}

void SSPHH_Application::imguiShowSphlEditor()
{
	///////////////////////////////////////////////////////////
	// S P H L   E D I T O R //////////////////////////////////
	///////////////////////////////////////////////////////////

	if (Interface.tools.showSphlEditor)
	{
		x += w + 64.0f;
		ImGui::SetNextWindowContentWidth(512);
		ImGui::SetNextWindowSize(ImVec2(512, 256));
		ImGui::SetNextWindowPos(ImVec2(screenWidth - 600, screenHeight - 300));
		x += w;
		ImGui::Begin("SSPL Editor");
		ImGui::Text("%d SSPLs", ssg.ssphhLights.size());
		if (ImGui::Button("Add SSPL"))
		{
			imguiSphlAdd();
		}
		int i = 0;
		const float stepSize = 0.1f;
		int whichToDelete = -1;
		for (auto &sspl : ssg.ssphhLights)
		{
			ostringstream id;
			id << "MS" << setfill('0') << setw(2) << i << " '" << sspl.name << "'";
			const void *ptr = &sspl;
			va_list args;
			const char *colors[4] = {"R", "G", "B", "Mono"};
			const ImVec4 mscolors[4] = {
				{1.0f, 0.0f, 0.0f, 1.0f},
				{0.0f, 1.0f, 0.0f, 1.0f},
				{0.0f, 0.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f}};
			float minValue = -5.0f;
			float maxValue = 5.0f;
			ImGui::PushID(i);
			if (ImGui::TreeNodeV(ptr, id.str().c_str(), args))
			{
				ImGui::SameLine();
				ImGui::Checkbox("", &sspl.enabled);
				ImGui::SameLine();
				if (ImGui::SmallButton("Delete"))
				{
					whichToDelete = i;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Recalc"))
				{
					sspl.dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("min"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, minValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("-1"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, -1.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("0"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, 0.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("?1"))
				{
					sspl.RandomizeCoefficient(-1, AllDegrees, AllBands, -1.0f, 1.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("?MAX"))
				{
					sspl.RandomizeCoefficient(-1, AllDegrees, AllBands, minValue, maxValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("1"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, 1.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("max"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, maxValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("? Coefs"))
				{
					sspl.randomize = true;
					sspl.dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("? Position"))
				{
					sspl.randomizePosition = true;
				}

				ImGui::InputText("Name", &sspl.name[0], 32);
				ImGui::Separator();
				ImGui::Text("Shadow build time: %.1fms", sspl.depthSphlMap.buildTime);
				int lastNumDegrees = sspl.maxDegree;
				ImGui::DragInt("Max Degree", &sspl.maxDegree, 0.1f, 0, 9);
				if (lastNumDegrees != sspl.maxDegree)
				{
					sspl.ChangeDegrees(sspl.maxDegree);
				}
				Vector4f position0 = sspl.position;
				ImGui::DragFloat4("Position", sspl.position.ptr(), stepSize, -100.0f, 100.0f);
				if (position0 != sspl.position)
				{
					sspl.depthSphlMap.dirty = true;
				}
				sspl.position.w = Fluxions::clamp(sspl.position.w, 0.0f, 1.0f);
				ImGui::DragFloat("E0", &sspl.E0, stepSize, 0.0f, 10000.0f);
				ImGui::DragFloat("Falloff Radius", &sspl.falloffRadius, stepSize, 0.0f, 1000.0f);

				ImGui::Text("status: dirty? %d / shadow map? %d", sspl.dirty ? 1 : 0, sspl.depthSphlMap.dirty ? 1 : 0);

				for (int j = 0; j < 4; j++)
				{
					ostringstream label;
					label << "Multispectral " << colors[j];
					ptr = &sspl.msph[j];
					if (ImGui::TreeNodeV(ptr, label.str().c_str(), args))
					{
						ImGui::SameLine();
						ImGui::TextColored(mscolors[j], label.str().c_str());
						ImGui::PushID(j);
						ImGui::SameLine();
						if (ImGui::SmallButton("min"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, minValue);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("-1"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, -1.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("0"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, 0.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("?"))
						{
							sspl.RandomizeCoefficient(j, AllDegrees, AllBands, -1.0f, 1.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("?MAX"))
						{
							sspl.RandomizeCoefficient(j, AllDegrees, AllBands, minValue, maxValue);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("1"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, 1.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("max"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, maxValue);
						}
						ImGui::PopID();

						for (int l = 0; l <= sspl.maxDegree; l++)
						{
							ImGui::PushID(l);
							for (int m = -l; m <= l; m++)
							{
								int lm = sspl.msph[j].getCoefficientIndex(l, m);
								float oldValue = sspl.msph[j].getCoefficient(l, m);
								ImGui::PushID(i * 100000 + j * 10000 + lm * 100);
								auto offset = sspl.msph[j].getCoefficientIndex(l, m);
								ostringstream label;
								label << "l = " << l << ", m = " << m << " (" << offset << ")"
									  << "##" << setw(2) << setfill('0') << i << j;

								if (ImGui::SmallButton("iso"))
								{
									sspl.IsolateCoefficient(j, l, m);
								}
								ImGui::PushItemWidth(64.0f);
								ImGui::SameLine();
								ImGui::DragFloat(label.str().c_str(), &sspl.msph[j][offset], stepSize, -5.0f, 5.0f);
								ImGui::PopItemWidth();
								ImGui::SameLine();
								if (ImGui::SmallButton("min"))
								{
									sspl.SetCoefficient(j, l, m, minValue);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("-1"))
								{
									sspl.SetCoefficient(j, l, m, -1.0f);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("0"))
								{
									sspl.SetCoefficient(j, l, m, 0.0f);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("?"))
								{
									sspl.RandomizeCoefficient(j, l, m, -1.0f, 1.0f);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("?MAX"))
								{
									sspl.RandomizeCoefficient(j, l, m, minValue, maxValue);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("1"))
								{
									sspl.SetCoefficient(j, l, m, 1.0f);
								}
								ImGui::SameLine();
								if (ImGui::SmallButton("max"))
								{
									sspl.SetCoefficient(j, l, m, maxValue);
								}

								if (0) //m == 0)
								{
									ImGui::PushID(-l);
									ImGui::SameLine();
									if (ImGui::SmallButton("min"))
									{
										sspl.SetCoefficient(j, l, AllBands, minValue);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("-1"))
									{
										sspl.SetCoefficient(j, l, AllBands, -1.0f);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("0"))
									{
										sspl.SetCoefficient(j, l, AllBands, 0.0f);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("?"))
									{
										sspl.RandomizeCoefficient(j, l, AllBands, -1.0f, 1.0f);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("?MAX"))
									{
										sspl.RandomizeCoefficient(j, l, AllBands, minValue, maxValue);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("1"))
									{
										sspl.SetCoefficient(j, l, AllBands, 1.0f);
									}
									ImGui::SameLine();
									if (ImGui::SmallButton("max"))
									{
										sspl.SetCoefficient(j, l, AllBands, maxValue);
									}
									ImGui::PopID();
								}
								ImGui::PopID();
								float newValue = sspl.msph[j].getCoefficient(l, m);
								if (newValue != oldValue)
								{
									sspl.dirty = true;
								}
							}
							ImGui::PopID();
						}
						ImGui::TreePop();
					}
					else
					{
						ImGui::SameLine();
						ImGui::TextColored(mscolors[j], label.str().c_str());
						ImGui::PushID(j);
						ImGui::SameLine();
						if (ImGui::SmallButton("min"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, minValue);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("-1"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, -1.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("0"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, 0.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("?"))
						{
							sspl.RandomizeCoefficient(j, AllDegrees, AllBands, minValue, maxValue);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("1"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, 1.0f);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("max"))
						{
							sspl.SetCoefficient(j, AllDegrees, AllBands, maxValue);
						}
						ImGui::PopID();
					}
				}

				ImGui::TreePop();
			}
			else
			{
				ImGui::SameLine();
				ImGui::Checkbox("", &sspl.enabled);
				ImGui::SameLine();
				if (ImGui::SmallButton("Delete"))
				{
					whichToDelete = i;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Recalc"))
				{
					sspl.dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("min"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, minValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("-1"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, -1.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("0"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, 0.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("?"))
				{
					sspl.RandomizeCoefficient(-1, AllDegrees, AllBands, minValue, maxValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("1"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, 1.0f);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("max"))
				{
					sspl.SetCoefficient(-1, AllDegrees, AllBands, maxValue);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("? Coefs"))
				{
					sspl.randomize = true;
					sspl.dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("? Position"))
				{
					sspl.randomizePosition = true;
				}
			}
			if (sspl.dirty || sspl.randomizePosition)
			{
				sspl.depthSphlMap.dirty = true;
			}
			ImGui::PopID();
			i++;
		}
		if (whichToDelete >= 0)
		{
			ssg.ssphhLights.erase(ssg.ssphhLights.begin() + whichToDelete);
		}
		ImGui::End();
	}
}

void SSPHH_Application::imguiShowMaterialEditor()
{
	///////////////////////////////////////////////////////////
	// M A T E R I A L   E D I T O R //////////////////////////
	///////////////////////////////////////////////////////////

	if (Interface.tools.showMaterialEditor)
	{
		x += w + 64.0f;
		ImGui::SetNextWindowContentWidth(w);
		ImGui::SetNextWindowPos(ImVec2(x, 64));
		ImGui::Begin("Material Editor");
		//if (Interface.mtls.mtlsCollapsed.size() != ssg.materials.size())
		//{
		//	Interface.mtls.mtlsCollapsed.resize(ssg.materials.size());
		//}

		ImGui::Checkbox("Maps", &Interface.mtls.showMaps);
		if (Interface.mtls.showMaps)
		{
			int i = 0;
			SEPARATOR_NEWLINE
			for (auto &mtllib : ssg.materials)
			{
				ImGui::Text("Mtllib %i: %s", mtllib.first, mtllib.second.name.c_str());
				for (auto &map : mtllib.second.maps)
				{
					ImGui::Text("map: %s", map.second.mapName.c_str());
				}
				i++;
			}
			NEWLINE_SEPARATOR
		}

		ImGui::Checkbox("Mtls", &Interface.mtls.showMtls);
		if (Interface.mtls.showMtls)
		{
			int i = 0;
			for (auto &mtllib : ssg.materials)
			{
				string mtllib_key = mtllib.second.fpi.fullfname;
				bool *mtllib_collapsed = nullptr;
				if (Interface.mtls.mtllibsCollapsed.find(mtllib_key) == Interface.mtls.mtllibsCollapsed.end())
				{
					Interface.mtls.mtllibsCollapsed[mtllib_key] = false;
				}
				mtllib_collapsed = &Interface.mtls.mtllibsCollapsed[mtllib_key];
				if (mtllib_collapsed == nullptr)
					continue;
				ImGui::TextColored(Colors.Red, "MtlLib %i", mtllib.first, mtllib.second.fpi.fullfname.c_str());
				ImGui::SameLine();
				ImGui::Checkbox(mtllib_key.c_str(), mtllib_collapsed);
				if (*mtllib_collapsed)
				{
					int j = 0;
					for (auto &mtl : mtllib.second.mtls)
					{
						string mtl_name = mtllib.second.mtls.GetNameFromHandle(mtl.first);
						string mtllib_mtl_key = mtllib_key + "!!" + mtl_name;
						bool *mtl_collapsed = nullptr;
						if (Interface.mtls.mtlsCollapsed.find(mtllib_mtl_key) == Interface.mtls.mtlsCollapsed.end())
						{
							Interface.mtls.mtlsCollapsed[mtllib_mtl_key] = false;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].Ka = mtl.second.Ka.ptr();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].Kd = mtl.second.Kd.ptr();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].Ks = mtl.second.Ks.ptr();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].Ke = mtl.second.Ke.ptr();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBm = &mtl.second.PBm;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBk = &mtl.second.PBk;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBior = &mtl.second.PBior;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBKdm = &mtl.second.PBKdm;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBKsm = &mtl.second.PBKsm;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].PBGGXgamma = &mtl.second.PBGGXgamma;
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].map_Kd = mtl.second.map_Kd.c_str();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].map_Ks = mtl.second.map_Ks.c_str();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].map_bump = mtl.second.map_bump.c_str();
							Interface.mtls.mtls_ptrs[mtllib_mtl_key].map_normal = mtl.second.map_normal.c_str();
						}
						mtl_collapsed = &Interface.mtls.mtlsCollapsed[mtllib_mtl_key];
						if (mtl_collapsed == nullptr)
							continue;

						ImGui::TextColored(Colors.Yellow, "Material Name: %i", mtl.first);
						ImGui::SameLine();
						ImGui::Checkbox(mtl_name.c_str(), mtl_collapsed);
						if (*mtl_collapsed)
						{
							auto pmtl = Interface.mtls.mtls_ptrs[mtllib_mtl_key];
							NEWLINE_SEPARATOR
							const float stepSize = 0.01f;
							const float minSize = -5.0f;
							const float maxSize = 5.0f;
#define JOIN(x) (x + ("##" + mtllib_mtl_key)).c_str()
							ImGui::DragFloat(JOIN("PBk"), pmtl.PBk, stepSize, 0.0f, 10.0f);
							ImGui::DragFloat(JOIN("PBior"), pmtl.PBior, stepSize, 0.0f, 2.5f);
							ImGui::DragFloat(JOIN("PBkdm"), pmtl.PBKdm, stepSize, -1.0f, 1.0f);
							ImGui::DragFloat(JOIN("PBksm"), pmtl.PBKsm, stepSize, -1.0f, 1.0f);
							ImGui::DragFloat(JOIN("PBGGXgamma"), pmtl.PBGGXgamma, stepSize, 0.0f, 1.0f);
							ImGui::DragFloat3(JOIN("Kd"), pmtl.Kd, stepSize, 0.0f, 1.0f);
							ImGui::Text(JOIN("map_Kd: %s"), pmtl.map_Kd);
							ImGui::DragFloat3(JOIN("Ks"), pmtl.Ks, stepSize, 0.0f, 1.0f);
							ImGui::Text(JOIN("map_Ks: %s"), pmtl.map_Ks);
							ImGui::Text(JOIN("map_bump: %s"), pmtl.map_bump);
							ImGui::Text(JOIN("map_normal: %s"), pmtl.map_normal);
#undef JOIN
						}
						j++;
						ImGui::Separator();
					}
					ImGui::Separator();
				}
				i++;
			}
		}
		ImGui::End();
	}
}

void SSPHH_Application::imguiSphlAdd()
{
	ssg.ssphhLights.resize(ssg.ssphhLights.size() + 1);
	SimpleSSPHHLight &sphl = ssg.ssphhLights.back();
	ostringstream name;
	name << "New SSPL " << ssg.ssphhLights.size();
	sphl.name = name.str();
	sphl.name.reserve(32);
	sphl.ChangeDegrees(DefaultSphlDegree);

	for (auto &sphl : ssg.ssphhLights)
	{
		sphl.dirty = true;
	}

	imguiCoronaGenerateSphlINIT();
}

void SSPHH_Application::imguiSphlDelete()
{
	if (!ssg.ssphhLights.empty())
		ssg.ssphhLights.pop_back();

	imguiCoronaGenerateSphlINIT();
}

void SSPHH_Application::imguiSphlRandomize()
{
	if (ssg.ssphhLights.empty())
		return;
	auto i = ssg.ssphhLights.size() - 1;
	ssg.ssphhLights[i].Randomize();
}

void SSPHH_Application::imguiSphlDecreaseDegrees()
{
	if (ssg.ssphhLights.empty())
		return;
	auto i = ssg.ssphhLights.size() - 1;
	ssg.ssphhLights[i].DecDegrees();
}

void SSPHH_Application::imguiSphlIncreaseDegrees()
{
	if (ssg.ssphhLights.empty())
		return;
	auto i = ssg.ssphhLights.size() - 1;
	ssg.ssphhLights[i].IncDegrees();
}

void SSPHH_Application::imguiSphlCoronaToSPH(size_t which)
{
	if (ssg.ssphhLights.empty() && ssg.ssphhLights.size() <= which)
		return;

	enviroCubeTexture3.LoadTextureCoronaCubeMap("export_cubemap.png", true);

	SphlSampler sphlSampler;
	sphlSampler.resize(64, 32);
	sphlSampler.saveSampleMap("sample_stratification.ppm", 4);
	sphlSampler.sampleCubeMap(enviroCubeTexture3.GetImage(), ssg.ssphhLights[which].msph);

	ssg.ssphhLights[which].dirty = true;
}

void SSPHH_Application::imguiSphlSPHtoCubeMap(size_t which)
{
	if (ssg.ssphhLights.empty() && ssg.ssphhLights.size() <= which)
		return;

	ssg.ssphhLights[which].dirty = true;
}

void SSPHH_Application::imguiSphlSaveToOBJ()
{
	int i = 0;
	for (auto &it : sphls)
	{
		auto &sphl = it.second;

		ostringstream ostr;
		ostr << "sphl" << setw(2) << setfill('0') << i;
		sphl.sph_model.SaveOBJ(ostr.str());
		i++;
	}
}

void SSPHH_Application::imguiToolsTakeScreenshot()
{
	Interface.saveScreenshot = true;
}

void SSPHH_Application::imguiToolsSaveStats()
{
	hflog.saveStats(ssg.name + "_");
}

void SSPHH_Application::imguiToolsResetStats()
{
	hflog.resetStat("frametime");
}

void SSPHH_Application::imguiCoronaControls()
{
	// TODO: Remove this control if we're not using it for anything anymore
	return;

	// 0x43524e41 = CRNA
	ImGui::PushID(0x43524e41);
	ImGui::LabelText("Corona", "");
	if (ImGui::Button("Generate SCN"))
	{
		imguiCoronaGenerateSCN();
	}
	if (ImGui::Button("SphlVIZ"))
	{
		imguiCoronaGenerateSphlVIZ();
	}
	ImGui::PopID();
}

void SSPHH_Application::imguiCoronaGenerateSCN()
{
	// Algorithm
	// generate export_corona_cubemap_XX.scn      (64x64) where XX is the index of the SPHL
	// generate export_corona_ground_truth.png (1280x720)
	// run corona to generate export_corona_cubemap.png      (64x64)
	// run corona to generate export_corona_ground_truth.png (1280x720)
	coronaScene.ClearCache();
	for (int i = 0; i < 16; i++)
	{
		if (i >= sphls.size())
			break;

		ostringstream ostr;
		ostr << "export_corona_cubemap_" << setw(2) << setfill('0') << i << ".scn";
		coronaScene.WriteCubeMapSCN(ostr.str(), ssg, sphls[i].position.xyz());
	}

	coronaScene.WriteSCN("export_corona_ground_truth.scn", ssg);
}

void SSPHH_Application::imguiCoronaGenerateSphlVIZ()
{
	imguiCoronaCheckCache();

	// Algorithm
	// for every pair of lights i, j where i != j,
	//     generate a SCN that represents the

	double viz_t0 = hflog.getSecondsElapsed();

	int count = 0;
	int numLights = (int)ssg.ssphhLights.size();
	Interface.ssphh.viz_times.resize(numLights, 0.0);

	for (int sendLight = 0; sendLight < numLights; sendLight++)
	{
		if (defaultRenderConfig.shaderDebugSphl >= 0 && sendLight != defaultRenderConfig.shaderDebugSphl)
			continue;

		auto &sphl = ssg.ssphhLights[sendLight];
		sphl.vizgenLightProbes.resize(numLights);

		for (int recvLight = 0; recvLight < numLights; recvLight++)
		{
			if (recvLight == sendLight)
				continue;

			int mrd = Interface.ssphh.VIZ_MaxRayDepth;
			int pl = Interface.ssphh.VIZ_PassLimit;
			string viz_name;
			viz_name = CoronaJob::MakeVIZName(Interface.sceneName,
											  recvLight,
											  sendLight,
											  false,
											  false,
											  Interface.ssphh.enableKs,
											  mrd,
											  pl);

			CoronaJob job(viz_name, CoronaJob::Type::VIZ, sendLight, recvLight);
			if (Interface.ssphh.enableHDR)
				job.EnableHDR();
			if (Interface.ssphh.enableHQ)
				job.EnableHQ();
			job.SetMaxRayDepth(mrd);
			job.SetPassLimit(mrd);
			job.SetIgnoreCache(Interface.ssphh.VIZ_IgnoreCache);
			job.SetImageDimensions(Interface.ssphh.LightProbeSize, Interface.ssphh.LightProbeSize);

			if (!ssphhUf.IsStopped())
			{
				// send it out
				ssphhUf.ScatterJob(job);
			}
			else
			{
				job.Start(coronaScene, ssg);
				GI_ProcessJob(job);
				count++;
			}
		}
	}

	if (count)
		ssg.ssphh.VIZ();

	Interface.ssphh.lastVIZTime = hflog.getSecondsElapsed() - viz_t0;
}

void SSPHH_Application::imguiCoronaGenerateSphlINIT()
{
	double t0 = hflog.getSecondsElapsed();
	ssg.ssphh.INIT(ssg);
	Interface.ssphh.lastINITTime = hflog.getSecondsElapsed() - t0;
	DirtySPHLs();
}

void SSPHH_Application::imguiCoronaGenerateSphlHIER()
{
	double t0 = hflog.getSecondsElapsed();
	ssg.ssphh.HIER(Interface.ssphh.HierarchiesIncludeSelf, Interface.ssphh.HierarchiesIncludeNeighbors, Interface.ssphh.MaxDegrees);
	Interface.ssphh.lastHIERTime = hflog.getSecondsElapsed() - t0;
	DirtySPHLs();
}

void SSPHH_Application::imguiCoronaGenerateSphlGEN()
{
	imguiCoronaCheckCache();

	// Algorithm
	// for every light i
	//     generate a SCN that represents the direct illumination from the location of the light
	//	   add contribution from neighboring lights
	//

	int count = 0;
	int numLights = (int)ssg.ssphhLights.size();
	double gen_t0 = hflog.getSecondsElapsed();
	Interface.ssphh.gen_times.resize(numLights, 0.0);
	for (int sendLight = 0; sendLight < numLights; sendLight++)
	{
		if (defaultRenderConfig.shaderDebugSphl >= 0 && sendLight != defaultRenderConfig.shaderDebugSphl)
			continue;
		auto &sphl = ssg.ssphhLights[sendLight];
		sphl.vizgenLightProbes.resize(numLights);

		//sphl.index = sendLight;
		int mrd = Interface.ssphh.GEN_MaxRayDepth;
		int pl = Interface.ssphh.GEN_PassLimit;
		CoronaJob job(CoronaJob::MakeGENName(
						  Interface.sceneName,
						  sendLight,
						  false,
						  false,
						  Interface.ssphh.enableKs,
						  mrd,
						  pl),
					  CoronaJob::Type::GEN, sendLight);
		if (Interface.ssphh.enableHDR)
			job.EnableHDR();
		if (Interface.ssphh.enableHQ)
			job.EnableHQ();
		job.SetMaxRayDepth(mrd);
		job.SetPassLimit(pl);
		job.SetImageDimensions(Interface.ssphh.LightProbeSize, Interface.ssphh.LightProbeSize);
		job.SetIgnoreCache(Interface.ssphh.GEN_IgnoreCache);

		if (!ssphhUf.IsStopped())
		{
			// send it out
			ssphhUf.ScatterJob(job);
		}
		else
		{
			job.Start(coronaScene, ssg);
			GI_ProcessJob(job);
			count++;
		}
	}

	if (count)
	{
		ssg.ssphh.GEN();
		DirtySPHLs();
	}

	Interface.ssphh.lastGENTime = hflog.getSecondsElapsed() - gen_t0;
}

void SSPHH_Application::imguiCoronaGenerateSky()
{
	CoronaJob job("ssphh_sky", CoronaJob::Type::Sky);

	if (Interface.ssphh.enableHQ)
		job.EnableHQ();
	if (Interface.ssphh.enableHDR)
		job.EnableHDR();

	job.Start(coronaScene, ssg);

	FilePathInfo fpi(job.GetOutputPath());
	if (fpi.Exists())
	{
		Image4f lightProbe;
		lightProbe.loadPPM(fpi.path);
		//lightProbe.scaleColors(1.0f / (2.5f * powf(2.0f, ssg.environment.toneMapExposure)));
		lightProbe.ReverseSRGB().ReverseToneMap(ssg.environment.toneMapExposure);
		lightProbe.convertRectToCubeMap();
		glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId);
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, lightProbe.width(), lightProbe.height(), 0, GL_RGBA, GL_FLOAT, lightProbe.getImageData(i));
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}

void SSPHH_Application::imguiCoronaCheckCache()
{
	if (coronaScene.enableKs != Interface.ssphh.enableKs)
	{
		coronaScene.enableKs = Interface.ssphh.enableKs;
		coronaScene.ClearCache();
	}
}

void SSPHH_Application::imguiCoronaGenerateREF()
{
	imguiCoronaCheckCache();
	if (Interface.ssphh.enableREF)
	{
		double t0 = hflog.getSecondsElapsed();
		string name = CoronaJob::MakeREFName(
			Interface.sceneName,
			false,
			Interface.ssphh.enableHDR,
			Interface.ssphh.enableHQ,
			Interface.ssphh.enableKs,
			Interface.ssphh.REF_MaxRayDepth,
			Interface.ssphh.REF_PassLimit);
		CoronaJob job1(name, CoronaJob::Type::REF);
		if (Interface.ssphh.enableHQ)
			job1.EnableHQ();
		if (Interface.ssphh.enableHDR)
			job1.EnableHDR();
		job1.SetMaxRayDepth(Interface.ssphh.REF_MaxRayDepth);
		job1.SetPassLimit(Interface.ssphh.REF_PassLimit);
		job1.SetIgnoreCache(Interface.ssphh.REF_IgnoreCache);
		// send it out
		if (!ssphhUf.IsStopped())
			ssphhUf.ScatterJob(job1);
		else
			job1.Start(coronaScene, ssg);
		Interface.ssphh.lastREFPath = job1.GetOutputPath();
		Interface.ssphh.lastREFTime = hflog.getSecondsElapsed() - t0;
	}

	if (Interface.ssphh.enableREFCubeMap)
	{
		double t0 = hflog.getSecondsElapsed();
		string name = CoronaJob::MakeREFName(
			Interface.sceneName,
			true,
			Interface.ssphh.enableHDR,
			Interface.ssphh.enableHQ,
			Interface.ssphh.enableKs,
			Interface.ssphh.REF_MaxRayDepth,
			Interface.ssphh.REF_PassLimit);
		CoronaJob job2(name, CoronaJob::Type::REF_CubeMap);
		if (Interface.ssphh.enableHQ)
			job2.EnableHQ();
		if (Interface.ssphh.enableHDR)
			job2.EnableHDR();
		job2.SetMaxRayDepth(Interface.ssphh.REF_MaxRayDepth);
		job2.SetPassLimit(Interface.ssphh.REF_PassLimit);
		job2.SetIgnoreCache(Interface.ssphh.REF_IgnoreCache);
		// send it out
		if (!ssphhUf.IsStopped())
			ssphhUf.ScatterJob(job2);
		else
			job2.Start(coronaScene, ssg);
		Interface.ssphh.lastREFCubeMapPath = job2.GetOutputPath();
		Interface.ssphh.lastREFCubeMapTime = hflog.getSecondsElapsed() - t0;
	}
}

void SSPHH_Application::imguiCoronaDeleteCache()
{
	for (int i = 0; i < MaxSphlLights; i++)
	{
		string base_filename = CoronaJob::MakeGENName(Interface.sceneName, i);

//vector<string> files_to_delete = {};
// Delete sun-to-sphl light probe
#ifdef WIN32
		DeleteFile((base_filename + ".scn").c_str());
		DeleteFile((base_filename + ".exr").c_str());
		DeleteFile((base_filename + ".ppm").c_str());
		DeleteFile((base_filename + "_hq.exr").c_str());
		DeleteFile((base_filename + "_hq.ppm").c_str());
		DeleteFile((base_filename + "_sph.ppm").c_str());
		DeleteFile((base_filename + ".json").c_str());
		DeleteFile((base_filename + "_tonemap.conf").c_str());
		DeleteFile((base_filename + "_01_Sprime.ppm").c_str());
		DeleteFile((base_filename + "_02_self.ppm").c_str());
		DeleteFile((base_filename + "_03_neighbor.ppm").c_str());
		DeleteFile((base_filename + "_01_Sprime.json").c_str());
		DeleteFile((base_filename + "_02_self.json").c_str());
		DeleteFile((base_filename + "_03_neighbor.json").c_str());
#endif
#ifdef __unix__
		unlink((base_filename + ".scn").c_str());
		unlink((base_filename + ".exr").c_str());
		unlink((base_filename + ".ppm").c_str());
		unlink((base_filename + "_hq.exr").c_str());
		unlink((base_filename + "_hq.ppm").c_str());
		unlink((base_filename + "_sph.ppm").c_str());
		unlink((base_filename + ".json").c_str());
		unlink((base_filename + "_tonemap.conf").c_str());
		unlink((base_filename + "_01_Sprime.ppm").c_str());
		unlink((base_filename + "_02_self.ppm").c_str());
		unlink((base_filename + "_03_neighbor.ppm").c_str());
		unlink((base_filename + "_01_Sprime.json").c_str());
		unlink((base_filename + "_02_self.json").c_str());
		unlink((base_filename + "_03_neighbor.json").c_str());
#endif

		// we count one more for the sun-to-sphl files
		for (int j = 0; j <= MaxSphlLights; j++)
		{
			string base_filename = CoronaJob::MakeVIZName(Interface.sceneName, i, j);

			// Delete sun-to-sphl light probe
			DeleteFile((base_filename + ".scn").c_str());
			DeleteFile((base_filename + ".exr").c_str());
			DeleteFile((base_filename + ".ppm").c_str());
			DeleteFile((base_filename + "_hq.exr").c_str());
			DeleteFile((base_filename + "_hq.ppm").c_str());
			DeleteFile((base_filename + "_sph.ppm").c_str());
			DeleteFile((base_filename + ".json").c_str());
			DeleteFile((base_filename + "_tonemap.conf").c_str());
			DeleteFile((base_filename + "_01_Sprime.ppm").c_str());
			DeleteFile((base_filename + "_02_self.ppm").c_str());
			DeleteFile((base_filename + "_03_neighbor.ppm").c_str());
			DeleteFile((base_filename + "_01_Sprime.json").c_str());
			DeleteFile((base_filename + "_02_self.json").c_str());
			DeleteFile((base_filename + "_03_neighbor.json").c_str());
		}
	}

	const string &gt = CoronaJob::MakeREFName(Interface.sceneName, false);
	const string &gt_cm = CoronaJob::MakeREFName(Interface.sceneName, true);
	const string &gt_mrdpl = CoronaJob::MakeREFName(Interface.sceneName, false, Interface.ssphh.enableHDR, false, Interface.ssphh.REF_MaxRayDepth, Interface.ssphh.REF_PassLimit);
	const string &gt_cm_mrdpl = CoronaJob::MakeREFName(Interface.sceneName, true, Interface.ssphh.enableHDR, false, Interface.ssphh.REF_MaxRayDepth, Interface.ssphh.REF_PassLimit);
	vector<string> products = {
		"ssphh_ground_truth",
		"ssphh_ground_truth_hq"
		"ssphh_ground_truth_cubemap",
		"ssphh_ground_truth_cubemap_hq",
		"ssphh_sky"
		"ssphh_sky_hq",
		"sky"
		"sky_hq",
		gt + "_hq",
		gt + "_hq_hdr",
		gt_cm + "_hq",
		gt_cm + "_hq_hdr",
		gt_mrdpl,
		gt_cm_mrdpl};

	for (auto &product : products)
	{
		DeleteFile((product + ".scn").c_str());
		DeleteFile((product + ".exr").c_str());
		DeleteFile((product + ".ppm").c_str());
		DeleteFile((product + ".png").c_str());
		DeleteFile((product + "_tonemap.conf").c_str());
		DeleteFile((product + "CESSENTIAL_Direct.exr").c_str());
		DeleteFile((product + "CESSENTIAL_Indirect.exr").c_str());
		DeleteFile((product + "CESSENTIAL_Reflect.exr").c_str());
	}
}

using TestProduct = tuple<bool, int, int, int, string>;

void SSPHH_Application::imguiCoronaGenerateTestProducts()
{
	vector<int> maxRayDepths = {25, 3, 1, 0};
	vector<int> passes = {1, 3, 25};
	vector<int> degrees = {9, 2};

	//// Exhaustive statistics
	//vector<int> maxRayDepths = { 25, 10, 5, 4, 3, 2, 1, 0 };
	//vector<int> passes = { 1, 2, 3, 4, 5, 10, 25 };
	//vector<int> degrees = { 9, 2 };

	//vector<int> maxRayDepths = { 25, 10, 5, 4, 3, 2, 1, 0 };
	//vector<int> passes = { 50, 25, 10, 5, 4, 3, 2, 1 };
	//vector<int> degrees = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	vector<int> specular = {1};
	map<TestProduct, double> times;

	auto gmrd = Interface.ssphh.GEN_MaxRayDepth;
	auto vmrd = Interface.ssphh.VIZ_MaxRayDepth;
	auto rmrd = Interface.ssphh.REF_MaxRayDepth;
	auto gpl = Interface.ssphh.GEN_PassLimit;
	auto vpl = Interface.ssphh.VIZ_PassLimit;
	auto rpl = Interface.ssphh.REF_PassLimit;
	auto gic = Interface.ssphh.GEN_IgnoreCache;
	auto vic = Interface.ssphh.VIZ_IgnoreCache;
	auto ric = Interface.ssphh.REF_IgnoreCache;
	auto lastDegrees = Interface.ssphh.MaxDegrees;

	int totalProducts = (int)(specular.size() * maxRayDepths.size() * passes.size() * degrees.size());
	int count = 0;

	// Render Ground Truth First
	coronaScene.ClearCache();
	coronaScene.enableKs = true;
	coronaScene.WriteCache(ssg);
	for (auto &mrd : maxRayDepths)
	{
		for (auto &pl : passes)
		{
			Interface.ssphh.REF_MaxRayDepth = mrd;
			Interface.ssphh.REF_PassLimit = pl;
			Interface.ssphh.REF_IgnoreCache = Interface.ssphh.genProductsIgnoreCache;
			Interface.ssphh.LightProbeSize = 32;
			Interface.ssphh.enableShowSPHLs = false;
			Interface.ssphh.enableShowHierarchies = false;
			Interface.ssphh.enableKs = true;

			string ptname = GetPathTracerName(Interface.sceneName, true, mrd, pl);
			hflog.infofn(__FUNCTION__, "Starting %s", ptname.c_str());

			coronaScene.WriteMaterials(ssg, true);
			imguiCoronaGenerateREF();
			times[make_tuple(true, mrd, pl, 0, "REF")] = Interface.ssphh.lastREFTime;
		}
	}

	for (auto &ks : specular)
	{
		coronaScene.ClearCache();
		coronaScene.enableKs = ks != 0;
		coronaScene.WriteCache(ssg);
		for (auto &mrd : maxRayDepths)
		{
			for (auto &pl : passes)
			{
				Interface.ssphh.GEN_MaxRayDepth = mrd;
				Interface.ssphh.GEN_PassLimit = pl;
				Interface.ssphh.GEN_IgnoreCache = Interface.ssphh.genProductsIgnoreCache;
				Interface.ssphh.VIZ_MaxRayDepth = mrd;
				Interface.ssphh.VIZ_PassLimit = pl;
				Interface.ssphh.VIZ_IgnoreCache = Interface.ssphh.genProductsIgnoreCache;
				Interface.ssphh.REF_MaxRayDepth = mrd;
				Interface.ssphh.REF_PassLimit = pl;
				Interface.ssphh.REF_IgnoreCache = Interface.ssphh.genProductsIgnoreCache;
				Interface.ssphh.LightProbeSize = 32;
				Interface.ssphh.enableShowSPHLs = false;
				Interface.ssphh.enableShowHierarchies = false;
				Interface.ssphh.enableKs = ks;

				string ptname = GetPathTracerName(Interface.sceneName, ks, mrd, pl);
				hflog.infofn(__FUNCTION__, "Starting %s", ptname.c_str());

				//coronaScene.WriteMaterials(ssg, true);
				//imguiCoronaGenerateREF();
				//times[make_tuple(ks, mrd, pl, 0, "REF")] = Interface.ssphh.lastREFTime;
				coronaScene.WriteMaterials(ssg, ks);
				imguiCoronaGenerateSphlINIT();
				times[make_tuple(ks, mrd, pl, 0, "INIT")] = Interface.ssphh.lastINITTime;
				if (mrd == 3 && pl == 1)
				{
					imguiCoronaGenerateSphlVIZ();
					times[make_tuple(ks, mrd, pl, 0, "VIZ")] = Interface.ssphh.lastVIZTime;
				}

				imguiCoronaGenerateSphlGEN();
				times[make_tuple(ks, mrd, pl, 0, "GEN")] = Interface.ssphh.lastGENTime;

				for (auto d : degrees)
				{

					ostringstream name;
					name << "HIER" << d;
					Interface.ssphh.MaxDegrees = d;
					imguiCoronaGenerateSphlHIER();
					times[make_tuple(ks, mrd, pl, d, name.str())] = Interface.ssphh.lastHIERTime;

					Interface.saveScreenshot = false;
					double frameTime = 0.0;
					for (int i = 0; i < 10; i++)
					{
						OnRender();
						frameTime += my_hud_info.totalRenderTime;
					}
					frameTime /= 10.0;

					Interface.saveScreenshot = true;
					OnRender();
					times[make_tuple(ks, mrd, pl, d, "sphlrender")] = frameTime;
					imguiCoronaGenerateCompareProduct(ks, mrd, pl, d);

					count++;
					float progress = (float)((double)count / (double)totalProducts);
					string pname = GetPathTracerSphlRenderName(Interface.sceneName, ks, mrd, pl, d);
					hflog.infofn(__FUNCTION__, "%4d/%4d (%3.2f%% done) products -- finished %s", count, totalProducts, progress,
								 pname.c_str());
				}
			}
		}
	}

	ofstream fout(Interface.sceneName + "_stats.csv",
				  Interface.ssphh.genProductsIgnoreCache ? ios::out : ios::app);
	auto dtg = hflog.makeDTG();
	fout << dtg << endl;
	fout << "name,dtg,ks,mrd,pl,md,time,ptE,sphlE,d1E,d2E" << endl;
	for (auto &product : times)
	{
		auto &t = product.first;
		auto time = product.second;
		string ks = std::get<0>(t) ? "Ks" : "None";
		auto &mrd = std::get<1>(t);
		auto &pl = std::get<2>(t);
		auto &md = std::get<3>(t);
		auto &name = std::get<4>(t);
		fout << name << "," << dtg << "," << ks << ", " << mrd << "," << pl << "," << md << "," << time << ",";
		fout << Interface.ssphh.lastPathTracerTotalEnergy << ",";
		fout << Interface.ssphh.lastSphlRenderTotalEnergy << ",";
		fout << Interface.ssphh.lastDiff1TotalEnergy << ",";
		fout << Interface.ssphh.lastDiff2TotalEnergy << endl;
	}
	fout.close();

	Interface.ssphh.GEN_MaxRayDepth = gmrd;
	Interface.ssphh.VIZ_MaxRayDepth = vmrd;
	Interface.ssphh.REF_MaxRayDepth = rmrd;
	Interface.ssphh.GEN_PassLimit = gpl;
	Interface.ssphh.VIZ_PassLimit = vpl;
	Interface.ssphh.REF_PassLimit = rpl;
	Interface.ssphh.GEN_IgnoreCache = gic;
	Interface.ssphh.VIZ_IgnoreCache = vic;
	Interface.ssphh.REF_IgnoreCache = ric;
	Interface.ssphh.MaxDegrees = lastDegrees;
}

void SSPHH_Application::imguiCoronaGenerateCompareProduct(bool ks, int mrd, int pl, int md)
{
	string pathtracer_name = GetPathTracerName(Interface.sceneName, ks, mrd, pl);
	string sphlrender_name = GetPathTracerSphlRenderName(Interface.sceneName, ks, mrd, pl, md);
	string stats_name = GetStatsName(Interface.sceneName, ks, mrd, pl, md);

	Image3f image1;
	Image3f image2;
	image1.loadPPM(Interface.ssphh.lastREFPath);
	image2.loadPPM(Interface.ssphh.lastSphlRenderPath);

	PPMCompare ppmcompare;

	ppmcompare.Init(ks, mrd, pl, md);
	ppmcompare.Compare(image1, image2);
	ppmcompare.SaveResults(Interface.sceneName, pathtracer_name,
						   Interface.ssphh.ppmcompareGenPPMs,
						   Interface.ssphh.ppmcompareIgnoreCache);

	Interface.ssphh.lastPathTracerTotalEnergy = ppmcompare.image1stat.sumI;
	Interface.ssphh.lastSphlRenderTotalEnergy = ppmcompare.image2stat.sumI;
	Interface.ssphh.lastDiff1TotalEnergy = ppmcompare.diffstat.sumI;
	Interface.ssphh.lastDiff2TotalEnergy = ppmcompare.absdiffstat.sumI;

	Interface.ssphh.ppmcompareIgnoreCache = false;
	return;

	//// do ppmcompare
	//ostringstream ostr;
	//ostr << Interface.sceneName;
	//if (ks) ostr << "_Ks";
	//ostr << "_" << setw(2) << setfill('0') << mrd;
	//ostr << "_" << setw(2) << setfill('0') << pl;
	//// ostr << "_" << setw(2) << setfill('0') << md;
	//string file1 = ostr.str();
	//string file2 = "render_";
	//char mdbuffer[17]; sprintf(mdbuffer, "%02d", md);
	//file2 += mdbuffer;

	//string file1ppm = file1 + ".ppm"; string file1png = file1 + "_00_pathtracer.png";
	//string file2ppm = file2 + ".ppm"; string file2png = (file1 + mdbuffer) + "_sphlrender.png";
	//if (!CopyFile(Interface.ssphh.lastREFPath.c_str(), file1ppm.c_str(), FALSE))
	//{
	//	hflog.error("%s(): Unable to copy file %s to %s", __FUNCTION__, Interface.ssphh.lastREFPath.c_str(), file1ppm.c_str());
	//	return;
	//}
	//if (!CopyFile(Interface.ssphh.lastSphlRenderPath.c_str(), file2ppm.c_str(), FALSE))
	//{
	//	hflog.error("%s(): Unable to copy file %s to %s", __FUNCTION__, Interface.ssphh.lastSphlRenderPath.c_str(), file2ppm.c_str());
	//	return;
	//}
	//char tmpbuf[256];
	//string name = ssg.name;
	//replace(name.begin(), name.end(), ' ', '_');
	//sprintf(tmpbuf, " -scene %s -mrd %d -pl %d -md %d %s %s", name.c_str(), mrd, pl, md, Interface.ssphh.ppmcompareIgnoreCache ? " -ignorecache" : "", Interface.ssphh.enableKs ? "-ks" : "");
	//string cmdline1 = "ppmcompare  "; cmdline1 += file1ppm + "  " + file2ppm + tmpbuf;
	//string cmdline2 = "magick  "; cmdline2 += file1ppm + "  " + file1png;
	//string cmdline3 = "magick  "; cmdline3 += file2ppm + "  " + file2png;

	//hflog.info("running: %s", cmdline1.c_str());
	//if (auto result = system(cmdline1.c_str()) != 0) hflog.error("%s(): command unsuccessful (%d) %s", __FUNCTION__, result, cmdline1.c_str());
	//if (auto result = system(cmdline2.c_str()) != 0) hflog.error("%s(): command unsuccessful (%d) %s", __FUNCTION__, result, cmdline2.c_str());
	//if (auto result = system(cmdline3.c_str()) != 0) hflog.error("%s(): command unsuccessful (%d) %s", __FUNCTION__, result, cmdline3.c_str());

	//DeleteFile(file1ppm.c_str());
	//DeleteFile(file2ppm.c_str());

	Interface.ssphh.ppmcompareIgnoreCache = false;
}

void SSPHH_Application::imguiCoronaDeleteTestProducts()
{
}

void SSPHH_Application::imguiUfShowWindow()
{
	if (!Interface.tools.showUnicornfishWindow)
		return;
	if (!Interface.uf.windowInit)
	{
		Interface.uf.windowInit = true;
		ImGui::SetNextWindowContentWidth(512.0f);
		ImGui::SetNextWindowPos(ImVec2(x + w + 32, 64));
	}
	ImGui::Begin("Unicornfish");
	if (Interface.uf.uf_type == UfType::None)
	{
		//if (ImGui::Button("UF CLIENT GLES30")) { Interface.uf.uf_type = UfType::Client; }
		//if (ImGui::Button("UF CLIENT GLES20")) { Interface.uf.uf_type = UfType::ClientGLES20; }
		//if (ImGui::Button("UF BROKER")) { Interface.uf.uf_type = UfType::Broker; }
		//if (ImGui::Button("UF WORKER ECHO")) { Interface.uf.uf_type = UfType::WorkerEcho; }
		//if (ImGui::Button("UF WORKER CORONA")) { Interface.uf.uf_type = UfType::Worker; }
		//ImGui::Separator();
		ImGui::Checkbox("Standalone CLIENT", &Interface.uf.standalone_client);
		ImGui::Checkbox("Standalone BROKER", &Interface.uf.standalone_broker);
		ImGui::Checkbox("Standalone WORKER", &Interface.uf.standalone_worker);
		if (ImGui::Button("START"))
		{
			ssphhUf.StartStandalone(
				Interface.uf.standalone_client,
				Interface.uf.standalone_broker,
				Interface.uf.standalone_worker);
		}
		ImGui::SameLine();
		if (ImGui::Button("STOP"))
		{
			ssphhUf.Stop();
			ssphhUf.Join();
		}
		ImGui::TextColored(Colors.Cyan, "CLIENT: %s", ssphhUf.GetMessage(SUFType::Client).c_str());
		ImGui::TextColored(Colors.Magenta, "BROKER: %s", ssphhUf.GetMessage(SUFType::Broker).c_str());
		ImGui::TextColored(Colors.Yellow, "WORKER: %s", ssphhUf.GetMessage(SUFType::Worker).c_str());
		ImGui::TextColored(Colors.Green, "GI STATUS: %s", Interface.ssphh.gi_status.c_str());
	}
	else
	{
		if (ImGui::Button("STOP"))
		{
			Interface.uf.uf_stop = true;
		}
		ImGui::Separator();

		switch (Interface.uf.uf_type)
		{
		case UfType::Broker:
			imguiUfBrokerControls();
			break;
		case UfType::ClientGLES20:
			imguiUfClientGLES20Controls();
			break;
		case UfType::Client:
			imguiUfClientGLES30Controls();
			break;
		case UfType::WorkerEcho:
			imguiUfWorkerEchoControls();
			break;
		case UfType::Worker:
			imguiUfWorkerCoronaControls();
			break;
		default:
			Interface.uf.uf_type = UfType::None;
		}
	}
	ImGui::End();
}

void SSPHH_Application::imguiShowSSPHHWindow()
{
	if (!Interface.tools.showSSPHHWindow)
		return;
	static bool init = false;
	if (!init)
	{
		float w = 480.0f;
		float h = 640.0f;
		ImGui::SetNextWindowPos(ImVec2(screenWidth - w, screenHeight - h));
		ImGui::SetNextWindowSize(ImVec2(w, h));
		init = true;
	}
	ImGui::SetNextWindowContentWidth(480.0f);
	ImGui::Begin("SSPHH");
	if (ImGui::Button("MV"))
	{
		init = false;
	}
	ImGui::SameLine();
	ImGui::Text("Scene: %s", Interface.sceneName.c_str());

	if (ImGui::Button("Generate Corona REF"))
	{
		imguiCoronaGenerateSCN();
	}
	if (ImGui::Button("HOSEK-WILKIE"))
	{
		imguiCoronaGenerateSky();
	}
	ImGui::SameLine();
	ImGui::Text("Create/use Corona Hosek-Wilkie sky.");

	if (ImGui::Button("REFERENCE"))
	{
		imguiCoronaGenerateREF();
	}
	ImGui::SameLine();
	ImGui::Text("Create reference for comparison (%.3lf sec)", Interface.ssphh.lastREFTime);

	if (ImGui::Button("DELETE CACHE"))
	{
		imguiCoronaDeleteCache();
	}
	ImGui::SameLine();
	ImGui::Text("Delete cached light solution");

	if (ImGui::Button("HIERGEN INIT"))
	{
		imguiCoronaGenerateSphlINIT();
	}
	ImGui::SameLine();
	ImGui::Text("Reset Hierarchies (%.3lf sec)", Interface.ssphh.lastINITTime);

	if (ImGui::Button("SPHLVIZ"))
	{
		imguiCoronaGenerateSphlVIZ();
	}
	ImGui::SameLine();
	ImGui::Text("Generate visibility network (%.3lf sec)", Interface.ssphh.lastVIZTime);

	if (ImGui::Button("SPHLGEN"))
	{
		imguiCoronaGenerateSphlGEN();
		ssg.MakeSphlsUnclean();
	}
	ImGui::SameLine();
	ImGui::Text("Generate GI solution (%.3lf sec)", Interface.ssphh.lastGENTime);

	if (ImGui::Button("HIERGEN"))
	{
		imguiCoronaGenerateSphlHIER();
		ssg.MakeSphlsUnclean();
	}
	ImGui::SameLine();
	ImGui::Text("Generate Hierarchies (%.3lf sec)", Interface.ssphh.lastHIERTime);

	if (ImGui::Button("SAVEOBJ"))
	{
		imguiSphlSaveToOBJ();
	}
	ImGui::SameLine();
	ImGui::Text("Save current SPHL(s) to OBJ/MTL");

	ImGui::Separator();

	ImGui::Checkbox("Sphl Editor", &Interface.tools.showSphlEditor);
	ImGui::PushID(1234);
	ImGui::SameLine();
	if (ImGui::SmallButton("+"))
	{
		imguiSphlAdd();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("-"))
	{
		imguiSphlDelete();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("l-"))
	{
		imguiSphlDecreaseDegrees();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("l+"))
	{
		imguiSphlIncreaseDegrees();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("?"))
	{
		imguiSphlRandomize();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Unclean!"))
	{
		ssg.MakeSphlsUnclean();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Dump Hier"))
	{
		for (int i = 0; i < ssg.ssphhLights.size(); i++)
		{
			auto &sphl = ssg.ssphhLights[i];
			sphl.SetHierarchyDescription();
			hflog.info("%s(): hierarchy %02d %s", __FUNCTION__, sphl.index, sphl.hier_description.c_str());
		}
	}

	// do a little visualization on the current enabled status
	int size = (int)ssg.ssphhLights.size();
	string bits(ssg.ssphhLights.size(), 'n');
	for (int i = 0; i < size; i++)
		if (ssg.ssphhLights[i].enabled)
			bits[i] = 'y';
	ImGui::SameLine();
	ImGui::Text("%d %s", size, bits.c_str());

	ImGui::PopID();

	ImGui::Separator();

	ImGui::Checkbox("Enable shadow map VIZ", &Interface.ssphh.enableShadowColorMap);
	ImGui::SameLine();
	ImGui::Checkbox("Enable sRGB", &defaultRenderConfig.enableSRGB);
	ImGui::Text("REF:");
	ImGui::SameLine();
	ImGui::Checkbox("2D", &Interface.ssphh.enableREF);
	ImGui::SameLine();
	ImGui::Checkbox("Cube", &Interface.ssphh.enableREFCubeMap);
	ImGui::SameLine();
	ImGui::Checkbox("HQ", &Interface.ssphh.enableHQ);
	ImGui::SameLine();
	ImGui::Checkbox("HDR", &Interface.ssphh.enableHDR);
	ImGui::SameLine();
	ImGui::Checkbox("Ks", &Interface.ssphh.enableKs);
	ImGui::SameLine();
	ImGui::Checkbox("PPMs", &ssg.ssphh.savePPMs);
	ImGui::SameLine();
	ImGui::Checkbox("JSONs", &ssg.ssphh.saveJSONs);

	ImGui::PushItemWidth(100);
	ImGui::PushID(1);
	ImGui::TextColored(Colors.Magenta, "VIZ");
	ImGui::SameLine();
	ImGui::SliderInt("MaxRayDepth", &Interface.ssphh.VIZ_MaxRayDepth, 0, 25);
	ImGui::SameLine();
	ImGui::SliderInt("PassLimit", &Interface.ssphh.VIZ_PassLimit, 1, 100);
	ImGui::SameLine();
	ImGui::Checkbox("Regen", &Interface.ssphh.VIZ_IgnoreCache);
	ImGui::PopID();
	ImGui::PushID(2);
	ImGui::TextColored(Colors.Cyan, "GEN");
	ImGui::SameLine();
	ImGui::SliderInt("MaxRayDepth", &Interface.ssphh.GEN_MaxRayDepth, 0, 25);
	ImGui::SameLine();
	ImGui::SliderInt("PassLimit", &Interface.ssphh.GEN_PassLimit, 1, 100);
	ImGui::SameLine();
	ImGui::Checkbox("Regen", &Interface.ssphh.GEN_IgnoreCache);
	ImGui::PopID();
	ImGui::PushID(3);
	ImGui::TextColored(Colors.Yellow, "REF");
	ImGui::SameLine();
	ImGui::SliderInt("MaxRayDepth", &Interface.ssphh.REF_MaxRayDepth, 0, 25);
	ImGui::SameLine();
	ImGui::SliderInt("PassLimit", &Interface.ssphh.REF_PassLimit, 1, 100);
	ImGui::SameLine();
	ImGui::Checkbox("Regen", &Interface.ssphh.REF_IgnoreCache);
	ImGui::PopID();
	imgui2NSizeSlider("SPHL Size", &Interface.ssphh.LightProbeSizeChoice, &Interface.ssphh.LightProbeSize, 4, 10);
	ImGui::SameLine();
	imgui2NSizeSlider("Shadow Size", &Interface.ssphh.ShadowSizeChoice, &Interface.ssphh.ShadowSize, 4, 10);
	ImGui::PopItemWidth();

	ImGui::Separator();

	ImGui::PushItemWidth(100);
	ImGui::SliderInt("Max Hierarchies", &Interface.ssphh.HierarchiesMaxSphls, 0, MaxSphlLights);
	ImGui::SameLine();
	ImGui::SliderInt("Max Degrees", &Interface.ssphh.MaxDegrees, 0, MaxSphlDegree);
	ImGui::Text("Accum");
	ImGui::SameLine();
	ImGui::Checkbox("Self ", &Interface.ssphh.HierarchiesIncludeSelf);
	ImGui::SameLine();
	ImGui::Checkbox("Neighbors", &Interface.ssphh.HierarchiesIncludeNeighbors);
	ImGui::Text("Show ");
	ImGui::SameLine();
	ImGui::Checkbox("SPHLs", &Interface.ssphh.enableShowSPHLs);
	ImGui::SameLine();
	ImGui::Checkbox("Basic", &Interface.ssphh.enableBasicShowSPHLs);
	ImGui::SameLine();
	ImGui::Checkbox("Hierarchies", &Interface.ssphh.enableShowHierarchies);
	ImGui::SameLine();
	if (ImGui::Button("GO!"))
	{
		ssg.MakeSphlsUnclean();
	}
	ImGui::PopItemWidth();

	ImGui::Separator();

	if (ImGui::Button("GEN Test Products"))
	{
		imguiCoronaGenerateTestProducts();
	}
	ImGui::SameLine();
	if (ImGui::Button("DEL Test Products"))
	{
		imguiCoronaDeleteTestProducts();
	}
	ImGui::SameLine();
	ImGui::Checkbox("REGEN Test Products", &Interface.ssphh.genProductsIgnoreCache);

	ImGui::Checkbox("PPMCOMP Diffs", &Interface.ssphh.ppmcompareGenPPMs);
	ImGui::SameLine();
	ImGui::Checkbox("PPMCOMP Regen", &Interface.ssphh.ppmcompareIgnoreCache);

	ImGui::Separator();

	// show sorted list of hierarchies
	for (int i = 0; i < ssg.ssphhLights.size(); i++)
	{
		ImGui::Text("%s", ssg.ssphhLights[i].hier_description.c_str());
	}

	ImGui::End();
}

void SSPHH_Application::imgui2NSizeSlider(const char *desc, int *choice, int *size, int minvalue, int maxvalue)
{
	ImGui::SliderInt(desc, choice, minvalue, maxvalue);
	ImGui::SameLine();
	ImGui::Text("= %d", 2 << *choice);
	*size = 2 << *choice;
}