// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// PhD Scalable Spherical Harmonics Hierarchies
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

#include "pch.hpp"

#define USE_GLFW 1
//#define USE_FREEGLUT 1

#ifdef _WIN32
#define _STDCALL_SUPPORTED

#ifndef FLUXIONS_NO_SDL
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
#endif

#pragma comment(lib, "fluxions.lib")
#pragma comment(lib, "fluxions-deps.lib")
#pragma comment(lib, "fluxions-gte.lib")

#pragma comment(lib, "viperfish.lib")
#pragma comment(lib, "unicornfish.lib")
#pragma comment(lib, "damselfish.lib")

#ifndef FLUXIONS_NO_OPENEXR
#pragma comment(lib, "Half-2_3.lib")
//#pragma comment(lib, "Iex-2_3")
//#pragma comment(lib, "IexMath-2_3")
#pragma comment(lib, "IlmImf-2_3.lib")
//#pragma comment(lib, "IlmImfUtil-2_3")
//#pragma comment(lib, "IlmThread-2_3")
//#pragma comment(lib, "Imath-2_3")
#endif

#ifdef _DEBUG
#pragma comment(lib, "zmq4d.lib")
#else
#pragma comment(lib, "zmq4.lib")
#endif // _DEBUG

#endif // _WIN32

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fluxions.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <regex>
#include <viperfish.hpp>
#include <unicornfish.hpp>
#include <ssphhapp.hpp>

#include <DragDrop.hpp>
#include <GLFW_template.hpp>
#include <GLUT_template.hpp>

#ifndef FLUXIONS_NO_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

extern void do_tests();

Vf::Widget::SharedPtr vfApp;
Vf::Widget::SharedPtr imguiPtr;

double g_distance = -10.0;
double xrot = 0.0;
double yrot = 0.0;

void InitApp();
void KillApp();

namespace Fluxions
{
	extern int test_fluxions_simple_property(int argc, char **argv);
}

namespace Df
{
	extern int test_PythonInterpreter(int argc, char **argv);
}

int main(int argc, char **argv)
{
	do_tests();

	Uf::Init();

	std::map<std::string, std::string> options = Fluxions::MakeOptionsFromArgs(argc, (const char **)argv);

	if (options.count("version")) {
		printf("SSPHH by Jonathan Metzgar\nCopyright (C) 2017 Jonathan Metzgar\n\n");
		printf("This program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\n");
		printf("This program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\n");
		printf("You should have received a copy of the GNU General Public License\nalong with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n");
	}

	Fluxions::debugging = false;
	if (options.count("debug")) {
		Fluxions::debugging = true;
	}

#ifdef USE_FREEGLUT
	GlutTemplateSetParameters("SSPHH", 1280, 720);
	GlutTemplateInit(argc, argv);
#endif
#ifdef USE_GLFW
	GlfwTemplateSetParameters("SSPHH", 1280, 720);
	if (!GlfwTemplateInit(argc, argv)) {
		return -1;
	}
#endif
	dragDrop.Init();
	InitApp();
#ifdef USE_FREEGLUT
	GlutTemplateWidget(vfApp);
	GlutTemplateMainLoop();
#endif
#ifdef USE_GLFW
	GlfwTemplateWidget(vfApp);
	GlfwTemplateMainLoop();
#endif
	KillApp();
	dragDrop.Kill();

	Uf::Kill();

	return 0;
}

// InitApp()
// This function is called prior to the main loop of the application.
// OpenGL is already initialized prior to this call.
//
void InitApp()
{
	Fluxions::Init();
	Fluxions::EnableGLDebugFunc();

	ssphhPtr = std::make_shared<SSPHH::SSPHH_Application>("ssphh");
	imguiPtr = std::make_shared<Vf::DearImGuiWidget>("imguiwidget");
	imguiPtr->decorate(ssphhPtr);
	vfApp = std::make_shared<Vf::Widget>("controller");
	vfApp->decorate(imguiPtr);
}

void KillApp()
{
	Fluxions::Kill();
}