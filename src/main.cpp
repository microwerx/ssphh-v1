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
#define _STDCALL_SUPPORTED

#ifndef FLUXIONS_NO_SDL
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_image.lib")
#endif

#pragma comment(lib, "fluxions.lib")
#pragma comment(lib, "glew32.lib")
#ifndef FLUXIONS_NO_OPENEXR
#pragma comment(lib, "Half-2_3.lib")
//#pragma comment(lib, "Iex-2_3")
//#pragma comment(lib, "IexMath-2_3")
#pragma comment(lib, "IlmImf-2_3.lib")
//#pragma comment(lib, "IlmImfUtil-2_3")
//#pragma comment(lib, "IlmThread-2_3")
//#pragma comment(lib, "Imath-2_3")
#endif

#ifdef NDEBUG
#pragma comment(lib, "freeglut_static.lib")
#pragma comment(lib, "zmq4.lib")
#else
#pragma comment(lib, "freeglut_staticd.lib")
#pragma comment(lib, "zmq4d.lib")
#endif // NDEBUG
#endif // WIN32

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

#include <glut-template.hpp>

#ifndef FLUXIONS_NO_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

extern void do_tests();

Viperfish::Widget::SharedPtr vfApp;
Viperfish::Widget::SharedPtr imguiPtr;

double g_distance = -10.0;
double xrot = 0.0;
double yrot = 0.0;

void Init();
void Kill();

namespace Fluxions
{
extern int test_fluxions_simple_property(int argc, char **argv);
}

namespace KASL
{
extern int test_PythonInterpreter(int argc, char **argv);
}

int main(int argc, char **argv)
{
	do_tests();

	curl_global_init(CURL_GLOBAL_ALL);

	map<string, string> options = Fluxions::MakeOptionsFromArgs(argc, (const char **)argv);

	if (options.count("version"))
	{
		printf("SSPHH by Jonathan Metzgar\nCopyright (C) 2017 Jonathan Metzgar\n\n");
		printf("This program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\n");
		printf("This program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\n");
		printf("You should have received a copy of the GNU General Public License\nalong with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n");
	}

	Fluxions::debugging = false;
	if (options.count("debug"))
	{
		Fluxions::debugging = true;
	}

	GlutTemplateSetParameters("SSPHH", 1280, 720);
	GlutTemplateInit(argc, argv);
	Init();
	GlutTemplateWidget(vfApp);
	GlutTemplateMainLoop();
	Kill();

	curl_global_cleanup();

	return 0;
}

// Init()
// This function is called prior to the main loop of the application.
// OpenGL is already initialized prior to this call.
//
void Init()
{
	Fluxions::Init();
	Fluxions::EnableGLDebugFunc();

	ssphhPtr = std::make_shared<SSPHH::SSPHH_Application>("ssphh");
	imguiPtr = std::make_shared<Viperfish::DearImGuiWidget>("imguiwidget");
	imguiPtr->decorate(ssphhPtr);
	vfApp = std::make_shared<Viperfish::Widget>("controller");
	vfApp->decorate(imguiPtr);
}

void Kill()
{
	Fluxions::Kill();
}
<<<<<<< HEAD

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
	string m(message, length);
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
=======
>>>>>>> e1246e9235f084527a11053e742c1294e6459f98
