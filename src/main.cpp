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

// For Drag and Drop
#include <Windows.h>
#include <WinUser.h>
#include <shellapi.h> // for DragAcceptFiles()
#include <oleidl.h>		// for Drag and Drop

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

void InitApp();
void KillApp();

namespace Fluxions
{
extern int test_fluxions_simple_property(int argc, char **argv);
}

namespace KASL
{
extern int test_PythonInterpreter(int argc, char **argv);
}

class MyDropTarget : public IDropTarget
{
private:
	ULONG referenceCount;
	bool acceptFormat;
	HWND window;

public:
	MyDropTarget()
	{
		window = GetActiveWindow();
		acceptFormat = false;
		referenceCount = 1;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
	{
		// tell other objects about our capabilities 
		if (riid == IID_IUnknown || riid == IID_IDropTarget)
		{
			*ppvObject = this;
			AddRef();
			return NOERROR;
		}
		*ppvObject = NULL;
		return ResultFromScode(E_NOINTERFACE);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return referenceCount++;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) override
	{
		if (--referenceCount == 0) {
			delete this;
			return 0;
		}
		return referenceCount;
	}

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		hflog.infofn(__FUNCTION__, "DragEnter");
		return NOERROR;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		hflog.infofn(__FUNCTION__, "DragOver");
		return NOERROR;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void) override
	{
		hflog.infofn(__FUNCTION__, "DragLeave");
		return NOERROR;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect) override
	{
		hflog.infofn(__FUNCTION__, "Drop");
		return NOERROR;
	}
};

static MyDropTarget *pDropTarget = nullptr;

void InitDragDrop()
{
	hflog.infofn(__FUNCTION__, "Starting Drag and Drop");
	HRESULT oleInitialized = OleInitialize(nullptr);
	HWND hwnd = GetActiveWindow();
	if (!hwnd) return;
	DragAcceptFiles(hwnd, TRUE);
	pDropTarget = new MyDropTarget();
	HRESULT result = RegisterDragDrop(hwnd, (IDropTarget *)pDropTarget);
}

void KillDragDrop()
{
	if (pDropTarget) {
		if (pDropTarget->Release() == 0) {
			pDropTarget = nullptr;
		};
	}
	OleUninitialize();
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

	GlutTemplateSetParameters("SSPHH", 2048, 1024);
	GlutTemplateInit(argc, argv);
	InitApp();
	GlutTemplateWidget(vfApp);
	InitDragDrop();
	GlutTemplateMainLoop();
	KillApp();

	curl_global_cleanup();

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
	imguiPtr = std::make_shared<Viperfish::DearImGuiWidget>("imguiwidget");
	imguiPtr->decorate(ssphhPtr);
	vfApp = std::make_shared<Viperfish::Widget>("controller");
	vfApp->decorate(imguiPtr);
}

void KillApp()
{
	Fluxions::Kill();
}