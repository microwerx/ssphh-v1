// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/KASL/GLUT Extensions
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

#include "stdafx.h"

#ifdef WIN32
#ifdef NDEBUG
#ifndef FLUXIONS_NO_SDL
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
#endif

#pragma comment(lib, "freeglut_static.lib")
#pragma comment(lib, "zmq4.lib")
#else

#ifndef FLUXIONS_NO_SDL
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
#endif

#pragma comment(lib, "freeglut_staticd.lib")
#pragma comment(lib, "zmq4d.lib")
#endif

#pragma comment(lib, "fluxions.lib")
#define _STDCALL_SUPPORTED
#endif

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
#include "ssphh.hpp"

#ifndef FLUXIONS_NO_SDL
#ifdef WIN32
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif
#endif

// #define USE_MULTISAMPLING

#ifndef WIN32
#define APIENTRY
#endif

using namespace std;
//using namespace SSPHH;

extern void do_tests();

void PrintString9x15(float x, float y, int justification, const char *format, ...);
void PrintBitmapStringJustified(float x, float y, int justification, void *font, const char *format, ...);
void PrintStrokeStringJustified(float x, float y, int justification, void *font, const char *format, ...);

#define GRAPP2017 0

#if GRAPP2017
const char *g_windowTitle = "GRAPP 2017 - Metzgar - Semwal";
int g_displayMode = GLUT_DOUBLE | GLUT_STENCIL | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_SRGB | GLUT_BORDERLESS;
int g_screenWidth = 1800;
int g_screenHeight = 1200;
#else
const char *g_windowTitle = "Scalable Spherical Harmonics Hierarchies (SSPHH)";
int g_displayMode = GLUT_DOUBLE | GLUT_STENCIL | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_SRGB; // | GLUT_BORDERLESS;
int g_screenWidth = 1280;
int g_screenHeight = 720;
#endif
double g_Fps = 0;
double g_aspectRatio = (double)g_screenWidth / (double)g_screenHeight;
int g_mouseDX = 0;
int g_mouseDY = 0;
int g_mouseX = 0;
int g_mouseY = 0;
int g_mouseButtons = 0;
int g_joystickButtonMask = 0;
int g_joystickX = 0;
int g_joystickY = 0;
int g_joystickZ = 0;
int g_joystickPollInterval = 16;
int g_timer1millis = 0;
int g_timer2millis = 0;
int g_timer3millis = 0;
int g_timer4millis = 0;
int g_timer1value = 0;
int g_timer2value = 0;
int g_timer3value = 0;
int g_timer4value = 0;
int g_frameCount = 0;
bool g_willResetErrorCountAfterFrame = false;
int g_errorCount = 0;
double g_updateInterval = 0.01666667f;
double g_renderInterval = 0.01666667f;
double g_Fov = 45.0;
double g_zNear = 0.01;
double g_zFar = 100.0;
int g_keyMap[256] = {0};
int g_specialKeyMap[256] = {0};
vector<string> g_args;

using namespace Viperfish;
Widget::SharedPtr vfApp;
Widget::SharedPtr imguiPtr;

double g_distance = -10.0;
double xrot = 0.0;
double yrot = 0.0;

void ParseCommandLine();
void InitMenu();
void OnInit();
void OnKill();
void OnUpdate(double deltaTime);
void OnRender();
void OnRenderOverlay();

void Set2DViewport();
void Set3DViewport();

void APIENTRY glutTemplateDebugFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

void glutCreateAffinity();

void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void special(int key, int x, int y);
void specialup(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void passive(int x, int y);
void entry(int state);
void visibility(int state);
void idle();
void timer1(int value);
void timer2(int value);
void timer3(int value);
void timer4(int value);
void menu(int state);
void menustatus(int state, int x, int y);
void windowstatus(int state);
void joystick(unsigned int buttonMask, int x, int y, int z);

namespace Fluxions
{
extern int test_fluxions_simple_property(int argc, char **argv);
}

namespace KASL
{
extern int test_PythonInterpreter(int argc, char **argv);
}

void glutCreateAffinity()
{
#if (0)
	glewInit();
	const int MAX_GPUS = 4;
	GPU_DEVICE gpuDevice;
	HGPUNV hGPU[MAX_GPUS];
	vector<GPU_DEVICE> gpuDevices;

	if (!glewIsExtensionSupported("WGL_NV_gpu_affinity"))
	{
		hflog.info("%s(): WGL_NV_gpu_affinity not supported", __FUNCTION__);
		return;
	}

	int i = 0;
	while (i < MAX_GPUS && wglEnumGpusNV(i, &hGPU[i]) == TRUE)
	{
		wglEnumGpuDevicesNV(hGPU[i], i, &gpuDevice);
		gpuDevices.push_back(gpuDevice);
		hflog.info("%s(): WGL_NV_gpu_affinity supports %d Device: %s Name: %s", __FUNCTION__, i, gpuDevice.DeviceName, gpuDevice.DeviceString);
		i++;
	}
#endif
}

std::map<std::string, std::string> MakeOptionsFromArgs(int argc, const char **argv)
{
	std::map<std::string, std::string> argv_options;
	for (int i = 0; i < argc; i++)
	{
		string option = argv[i];
		hflog.info("%s(): Processing '%s'", __FUNCTION__, option.c_str());
		std::regex dashequals("(^-+|=)",
							  std::regex_constants::ECMAScript);
		std::sregex_token_iterator it(option.begin(),
									  option.end(),
									  dashequals,
									  -1);
		std::sregex_token_iterator end;
		size_t count = 0;

		string key = "";
		string value = "";
		for (; it != end; it++)
		{
			if (count == 1)
			{
				key = *it;
			}
			else if (count == 2)
			{
				value = *it;
			}
			string token = *it;
			hflog.info("%s(): token %d '%s'", __FUNCTION__, count, token.c_str());
			count++;
		}
		if (key.length() > 0)
		{
			argv_options.emplace(std::make_pair(key, value));
			hflog.info("%s(): argv adding key '%s' = '%s'", __FUNCTION__, key.c_str(), value.c_str());
		}
	}
	return argv_options;
}

int main(int argc, char **argv)
{
	do_tests();

	//return Fluxions::test_fluxions_simple_property(argc, argv);
	//return KASL::test_PythonInterpreter(argc, argv);

	curl_global_init(CURL_GLOBAL_ALL);

	bool showVersion = false;
	map<string, string> options = MakeOptionsFromArgs(argc, (const char **)argv);
	if (options.count("version"))
	{
		showVersion = true;
	}

	if (showVersion)
	{
		printf("SSPHH by Jonathan Metzgar\nCopyright (C) 2017 Jonathan Metzgar\n\n");
		printf("This program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\n");
		printf("This program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\n");
		printf("You should have received a copy of the GNU General Public License\nalong with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n");
	}

#ifdef WIN32
	SetProcessDPIAware();
#endif

	for (int i = 0; i < argc; i++)
	{
		g_args.push_back(argv[i]);
	}

	glutInit(&argc, argv);
#ifdef USE_MULTISAMPLING
	glutSetOption(GLUT_MULTISAMPLE, 4);
#endif
	glutInitDisplayMode(g_displayMode);
	glutInitWindowSize(g_screenWidth, g_screenHeight);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextFlags(GLUT_DEBUG);
	glutCreateWindow(g_windowTitle);
	// glutCreateAffinity();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive);
	glutEntryFunc(entry);
	glutVisibilityFunc(visibility);
	glutIdleFunc(idle);
	if (g_timer1millis != 0)
		glutTimerFunc(g_timer1millis, timer1, g_timer1value);
	if (g_timer2millis != 0)
		glutTimerFunc(g_timer2millis, timer2, g_timer2value);
	if (g_timer3millis != 0)
		glutTimerFunc(g_timer3millis, timer3, g_timer3value);
	if (g_timer4millis != 0)
		glutTimerFunc(g_timer4millis, timer4, g_timer4value);
	glutMenuStateFunc(menu);
	glutMenuStatusFunc(menustatus);
	glutWindowStatusFunc(windowstatus);
	glutJoystickFunc(joystick, g_joystickPollInterval);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	try
	{
		ParseCommandLine();
		InitMenu();
		OnInit();
		glutMainLoop();
		OnKill();
	}
	catch (...)
	{
		hflog.error("%s(): Caught an unhandled exception", __FUNCTION__);
	}

	curl_global_cleanup();

	return 0;
}

// ParseCommandLine()
// This function is called prior to the OnInit() initialization call of the
// application.
void ParseCommandLine()
{
	for (auto arg = g_args.begin(); arg != g_args.end(); arg++)
	{
		// do something with the arguments...
	}
}

// InitMenu()
void InitMenu()
{
	// Initialize menu entries here
}

// OnInit()
// This function is called prior to the main loop of the application.
// OpenGL is already initialized prior to this call.
//
void OnInit()
{
	hflog.info("%s(): Initializing OpenGL", __FUNCTION__);
	glewInit();

#ifdef USE_MULTISAMPLING
	glEnable(GL_MULTISAMPLE);
#endif
	std::cerr << std::hex << glDebugMessageCallback << std::endl;
	if (glDebugMessageCallback)
	{
		glDebugMessageCallback((GLDEBUGPROC)glutTemplateDebugFunc, NULL);
		glEnable(GL_DEBUG_OUTPUT);
	}
	else
	{
		hflog.warningfn(__FUNCTION__, "No glDebugMessageCallback");
	}

	const char *versionString = (char *)glGetString(GL_VERSION);
	const char *vendorString = (char *)glGetString(GL_VENDOR);
	const char *rendererString = (char *)glGetString(GL_RENDERER);
	const char *extensionsString = (char *)glGetString(GL_EXTENSIONS);

	bool showglextension = false;
	for (auto arg = g_args.begin(); arg != g_args.end(); arg++)
	{
		if (*arg == "--glextensions")
		{
			showglextension = true;
		}
	}

	hflog.info("GL_RENDERER: %s\n", rendererString);
	hflog.info("GL_VERSION:  %s\n", versionString);
	hflog.info("GL_VENDOR:   %s\n", vendorString);
	if (showglextension)
	{
		hflog.info("GL_EXTENSIONS: %s\n", extensionsString);
	}

	GLint maxUniformBufferBindings;

	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
	hflog.info("GL_MAX_UNIFORM_BUFFER_BINDINGS: %d\n", maxUniformBufferBindings);

	glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB, GL_NICEST);
#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES
	glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES, GL_NICEST);
#endif

#ifndef FLUXIONS_NO_SDL
	hflog.info("%s(): Initializing SDL", __FUNCTION__);
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG || IMG_INIT_PNG);
#endif

	hflog.info("%s(): Initializing SSPHH");
	//InitSSPHH();
	ssphhPtr = MakeShared<SSPHH_Application>();
	imguiPtr = ImGuiWidget::MakeShared();
	vfApp = Widget::MakeShared();

	Widget::SharedPtr wssphhPtr = ssphhPtr;
	Widget::SharedPtr wimguiPtr = imguiPtr;
	imguiPtr->decorate(wssphhPtr);
	vfApp->decorate(wimguiPtr);

	hflog.info("%s(): Initializing application", __FUNCTION__);
	//ssphhPtr->Init(g_args);
	//ssphhPtr->OnInit(g_args);
	vfApp->Init(g_args);
	//vfApp->OnInit(g_args);
}

void OnKill()
{
	KillSSPHH();

	//IMG_Quit();
	//SDL_Quit();
}

// OnUpdate(deltaTime)
// This function is called during the main loop of the application.
//
void OnUpdate(double deltaTime)
{
	xrot += 35 * deltaTime;
	yrot += 35 * deltaTime;

#ifdef WIN32
	INPUT input;
	ZeroMemory(&input, sizeof(INPUT));
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(INPUT));
	Sleep(0);
#elif _POSIX_VERSION
	usleep(0);
#endif

	// ssphh.OnUpdate(deltaTime);
	vfApp->OnUpdate(deltaTime);
}

void OnRender()
{
	// RenderOpenGL11TestScene();

	// ssphh.OnRender();
	vfApp->OnRender();
}

void OnRenderOverlay()
{
	glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

	// ssphh.OnRenderOverlay();
	vfApp->OnRenderOverlay();
}

void Set2DViewport()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix4f ortho2d;
	ortho2d.Ortho2D(0, g_screenWidth, g_screenHeight, 0);
	glMultMatrixf(ortho2d.const_ptr());
	glMatrixMode(GL_MODELVIEW);
}

void Set3DViewport()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix4f perspective;
	perspective.PerspectiveY(g_Fov, g_aspectRatio, g_zNear, g_zFar);
	glMultMatrixf(perspective.const_ptr());
	glMatrixMode(GL_MODELVIEW);
}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, g_screenWidth, g_screenHeight);
	glLoadIdentity();
	Set3DViewport();
	OnRender();

	glViewport(0, 0, g_screenWidth, g_screenHeight);
	glLoadIdentity();
	Set2DViewport();
	OnRenderOverlay();

	if (g_displayMode & GLUT_DOUBLE)
	{
		glutSwapBuffers();
	}
	g_frameCount++;
	if (g_willResetErrorCountAfterFrame)
		g_errorCount = 0;
}

void reshape(int width, int height)
{
	g_aspectRatio = (double)width / (double)height;
	g_screenWidth = width;
	g_screenHeight = height;
	glViewport(0, 0, width, height);

	// ssphh.OnReshape(width, height);
	vfApp->OnReshape(width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	g_keyMap[key] = 1;

	string keyName = Viperfish::KeyToHTML5Name(key);
	// ssphh.OnKeyDown(keyName, glutGetModifiers());
	vfApp->OnKeyDown(keyName, glutGetModifiers());

	//if (key == 27)
	//	glutLeaveMainLoop();
	//if (key == '=')
	//	g_distance += 1;
	//if (key == '-')
	//	g_distance -= 1;
}

void keyboardup(unsigned char key, int x, int y)
{
	g_keyMap[key] = 0;

	string keyName = Viperfish::KeyToHTML5Name(key);
	// ssphh.OnKeyUp(keyName, glutGetModifiers());
	vfApp->OnKeyUp(keyName, glutGetModifiers());
}

void special(int key, int x, int y)
{
	g_specialKeyMap[key] = 1;

	string keyName = Viperfish::SpecialKeyToHTML5Name(key);
	// ssphh.OnKeyDown(keyName, glutGetModifiers());
	vfApp->OnKeyDown(keyName, glutGetModifiers());
}

void specialup(int key, int x, int y)
{
	g_specialKeyMap[key] = 0;

	string keyName = Viperfish::SpecialKeyToHTML5Name(key);
	// ssphh.OnKeyUp(keyName, glutGetModifiers());
	vfApp->OnKeyUp(keyName, glutGetModifiers());
}

void mouse(int button, int state, int x, int y)
{
	int buttonMask = 1 << button;
	if (state == GLUT_UP)
	{
		g_mouseButtons &= ~buttonMask;
	}
	else
	{
		g_mouseButtons |= buttonMask;
	}
	g_mouseDX = x - g_mouseX;
	g_mouseDY = x - g_mouseY;
	g_mouseX = x;
	g_mouseY = y;

	// ssphh.OnMouseMove(g_mouseX, g_mouseY);
	vfApp->OnMouseMove(g_mouseX, g_mouseY);
	if (state == GLUT_DOWN)
		// ssphh.OnMouseButtonDown(button);
		vfApp->OnMouseButtonDown(button);
	else if (state == GLUT_UP)
		// ssphh.OnMouseButtonUp(button);
		vfApp->OnMouseButtonUp(button);
}

void motion(int x, int y)
{
	g_mouseDX = x - g_mouseX;
	g_mouseDY = x - g_mouseY;
	g_mouseX = x;
	g_mouseY = y;

	// ssphh.OnMouseMove(g_mouseX, g_mouseY);
	vfApp->OnMouseMove(g_mouseX, g_mouseY);
}

void passive(int x, int y)
{
	g_mouseDX = x - g_mouseX;
	g_mouseDY = x - g_mouseY;
	g_mouseX = x;
	g_mouseY = y;

	// ssphh.OnMouseMove(X, y);
	vfApp->OnMouseMove(x, y);
}

void entry(int state)
{
	if (state == GLUT_ENTERED)
		// ssphh.OnMouseEnter();
		vfApp->OnMouseEnter();
	else
		// ssphh.OnMouseLeave();
		vfApp->OnMouseLeave();
}

void visibility(int state)
{
	if (state == GLUT_VISIBLE)
		// ssphh.OnWindowVisible();
		vfApp->OnWindowVisible();
	else if (state == GLUT_NOT_VISIBLE)
		// ssphh.OnWindowHidden();
		vfApp->OnWindowHidden();
}

void idle()
{
	static double time0 = 0, time1 = 0;
	static double renderdt = 0;
	static double updatedt = 0;
	static int fps_count = 0;
	static double fps0 = 0, fps1 = 0;

	time0 = time1;
	time1 = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	renderdt += time1 - time0;
	updatedt += time1 - time0;

	if (fps_count == 20)
	{
		fps0 = fps1;
		fps1 = time1;
		fps_count = 0;
	}

	g_Fps = 20.0 / (fps1 - fps0);

	if (updatedt > g_updateInterval)
	{
		OnUpdate(updatedt);
		// ssphh.OnUpdate(updatedt);
		vfApp->OnUpdate(updatedt);
		updatedt = 0;
	}

	if (renderdt > g_renderInterval)
	{
		//wglSwapIntervalEXT(-1);
		fps_count++;
		glutPostRedisplay();
		renderdt = 0;
	}
}

void timer1(int value)
{
}

void timer2(int value)
{
}

void timer3(int value)
{
}

void timer4(int value)
{
}

void menu(int state)
{
}

void menustatus(int state, int x, int y)
{
}

void windowstatus(int state)
{
	switch (state)
	{
	case GLUT_HIDDEN:
		break;
	case GLUT_FULLY_RETAINED:
		break;
	case GLUT_PARTIALLY_RETAINED:
		break;
	case GLUT_FULLY_COVERED:
		break;
	}
}

void joystick(unsigned int buttonMask, int x, int y, int z)
{
	g_joystickButtonMask = buttonMask;
	g_joystickX = x;
	g_joystickY = y;
	g_joystickZ = z;
}

// -- EXTRA FUNCTIONS ------------------------------------------------

void PrintString9x15(float x, float y, int justification, const char *format, ...)
{
	char buffer[256];

	va_list va;
	va_start(va, format);
#ifdef __STDC_SECURE_LIB__
	vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, format, va);
#else
	vsnprintf(buffer, 100, format, va);
#endif
	va_end(va);

	PrintBitmapStringJustified(x, y, justification, GLUT_BITMAP_9_BY_15, buffer);
}

void PrintBitmapStringJustified(float x, float y, int justification, void *font, const char *format, ...)
{
	char buffer[256];
	int pixelWidthOfString;

	va_list va;
	va_start(va, format);
#ifdef __STDC_SECURE_LIB__
	vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, format, va);
#else
	vsnprintf(buffer, 100, format, va);
#endif
	va_end(va);

	pixelWidthOfString = glutBitmapLength(font, (const unsigned char *)buffer);

	if (justification == LEFT)
	{
		// left justified
		glRasterPos2f(x, y);
	}
	else if (justification == RIGHT)
	{
		// right justified
		glRasterPos2f((GLfloat)(g_screenWidth - pixelWidthOfString), (GLfloat)y);
	}
	else if (justification == CENTER)
	{
		glRasterPos2f((GLfloat)(g_screenWidth - pixelWidthOfString) / 2.0f, (GLfloat)y);
	}

	glutBitmapString(font, buffer);
}

void PrintStrokeStringJustified(float x, float y, int justification, void *font, const char *format, ...)
{
	char buffer[2048];
	int pixelWidthOfString;

	va_list va;
	va_start(va, format);
#ifdef __STDC_SECURE_LIB__
	vsnprintf_s(buffer, sizeof(buffer), 2048, format, va);
#else
	vsnprintf(buffer, 2048, format, va);
#endif
	va_end(va);

	pixelWidthOfString = glutStrokeLength(font, (const unsigned char *)buffer);

	if (justification == LEFT)
	{
		// left justified
		glRasterPos2f(x, y);
	}
	else if (justification == RIGHT)
	{
		// right justified
		glRasterPos2f((float)(g_screenWidth - pixelWidthOfString), (float)y);
	}
	else if (justification == CENTER)
	{
		glRasterPos2f((float)(g_screenWidth - pixelWidthOfString) / 2.0f, (float)y);
	}

	glutStrokeString(font, buffer);
}

void APIENTRY glutTemplateDebugFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
	if (id == 131185)
		return;

	ostringstream ostr;
	ostr << "(" << g_CurrentDebugMessage << ") ";
	ostr << "frame: " << g_frameCount;
	ostr << " error: " << g_errorCount++;
	ostr << " id: " << id;

	ostr << " severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		ostr << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		ostr << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		ostr << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		ostr << "NOTIFICATION";
		break;
	default:
		ostr << severity;
		break;
	}

	ostr << " source: ";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		ostr << "API";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		ostr << "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		ostr << "OTHER";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		ostr << "SHADER COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		ostr << "THIRD PARTY";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		ostr << "WINDOW SYSTEM";
		break;
	default:
		ostr << type;
		break;
	}

	ostr << " type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		ostr << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		ostr << "DEPRECATED BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		ostr << "UNDEFINED BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		ostr << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		ostr << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_MARKER:
		ostr << "MARKER";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		ostr << "PUSH GROUP";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		ostr << "POP GROUP";
		break;
	case GL_DEBUG_TYPE_OTHER:
		ostr << "OTHER";
		break;
	default:
		ostr << type;
		break;
	}

	hflog.info("%s", ostr.str().c_str());
	string m = message;
	istringstream istr(m);
	string line;
	while (getline(istr, line))
	{
		hflog.info("> %s", line.c_str());
	}
}

void RenderCheckerboard()
{
	for (int i = -4; i < 4; i++)
	{
		for (int j = -4; j < 4; j++)
		{
			glNormal3f(0, 1, 0);
			if ((j % 2 && i % 2) || j == i || (!(j % 2) && !(i % 2)))
				glColor3f(.7f, .7f, .7f);
			else
				glColor3f(.3f, .3f, .3f);
			glRectf(2.0f * i, 2.0f * j, 2.0f * (i + 1), 2.0f * (j + 1));
		}
	}
}

void RenderOpenGL11TestScene()
{
	glColor3f(0.8f, 0.3f, 0.2f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glColor3f(0, 0, 1);
	glPushMatrix();
	glLoadIdentity();

	Matrix4f invCamera = ssphhPtr->Interface.inversePreCameraMatrix;
	glLoadMatrixf(invCamera.const_ptr());
	//glTranslatef(0, 0, -20 - g_distance);
	//glRotatef(yrot, 0.0, 1.0, 0.0);
	glutSolidTeapot(1.0);
	glPopMatrix();

	glColor3f(1, 0, 0);
	glLoadIdentity();
	//glTranslatef(0, -4, -20 - g_distance);
	glLoadMatrixf(invCamera.const_ptr());
	glTranslatef(0, -4, 0);
	glRotatef(-90, 1, 0, 0);
	glRectf(-1, -1, 1, 1);

	RenderCheckerboard();

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
}