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
#include "ssphh.hpp"
#include <memory>

namespace Fluxions {
extern bool debugging;
}

shared_ptr<SSPHH_Application> ssphhPtr;

extern vector<string> g_args;

using namespace Fluxions;

void InitSSPHH()
{
    ssphhPtr = std::make_shared<SSPHH_Application>();
}

void KillSSPHH()
{
    //ssphh->Stop();
    ssphhPtr->Kill();
}

extern void PrintString9x15(float x, float y, int justification, const char* format, ...);

SSPHH_Application::SSPHH_Application()
    : PBSkyCubeMap(GL_TEXTURE_CUBE_MAP)
{
    sceneFilename = "resources/scenes/test_texture_scene/test_gallery3_scene.scn";
}

SSPHH_Application::~SSPHH_Application()
{
}

void SSPHH_Application::ResetScene()
{
    ssg.environment.pbsky.SetCivilDateTime(ssg.environment.pbsky_dtg);
    ssg.environment.pbsky.ComputeSunFromLocale();
    pbsky_localtime = ssg.environment.pbsky.GetTime();
    pbsky_timeOffsetInSeconds = 0.0;
    Interface.recomputeSky = true;
    AdvanceSunClock(0.0, true);
    ssg.environment.ComputePBSky();
}

void SSPHH_Application::UseCurrentTime()
{
    pbsky_localtime = time(NULL);
    pbsky_timeOffsetInSeconds = 0.0;

    ssg.environment.pbsky.SetTime(time(NULL), 0.0);
    ssg.environment.pbsky.ComputeSunFromLocale();
    Interface.recomputeSky = true;
    ssg.environment.ComputePBSky();
}

void SSPHH_Application::InitRenderConfigs()
{
    glutSetErrorMessage(__FILE__, __LINE__, "inside InitRenderConfigs()");

    rectShadowRenderConfig.clearColorBuffer = true;
    rectShadowRenderConfig.clearDepthBuffer = true;
    rectShadowRenderConfig.viewportRect.x = 0;
    rectShadowRenderConfig.viewportRect.y = 0;
    rectShadowRenderConfig.viewportRect.w = Interface.renderconfig.sunShadowMapSize;
    rectShadowRenderConfig.viewportRect.h = Interface.renderconfig.sunShadowMapSize;
    rectShadowRenderConfig.fov = 90.0;
    rectShadowRenderConfig.isCubeMap = false;
    rectShadowRenderConfig.useSceneCamera = false;
    rectShadowRenderConfig.useZOnly = true;
    rectShadowRenderConfig.recomputeProjectionMatrix = false;
    rectShadowRenderConfig.fbo.Delete();
    rectShadowRenderConfig.fbo.SetDimensions(rectShadowRenderConfig.viewportRect.w, rectShadowRenderConfig.viewportRect.h);
    rectShadowRenderConfig.fbo.AddTexture2D(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GL_RGBA8, true);
    rectShadowRenderConfig.fbo.AddTexture2D(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, false);
    if (!rectShadowRenderConfig.fbo.Make()) {
        hflog.error("%s(): Could not make rect shadow map FBO.", __FUNCTION__);
    }

    cubeShadowRenderConfig.clearColorBuffer = true;
    cubeShadowRenderConfig.clearDepthBuffer = true;
    cubeShadowRenderConfig.viewportRect.x = 0;
    cubeShadowRenderConfig.viewportRect.y = 0;
    cubeShadowRenderConfig.viewportRect.w = 64;
    cubeShadowRenderConfig.viewportRect.h = 64;
    cubeShadowRenderConfig.isCubeMap = true;
    cubeShadowRenderConfig.useSceneCamera = false;
    cubeShadowRenderConfig.useZOnly = true;
    cubeShadowRenderConfig.fov = 90.0;
    cubeShadowRenderConfig.recomputeProjectionMatrix = false;
    cubeShadowRenderConfig.fbo.Delete();

    // new method with the Simple GLES 30 Renderer
    int cubeMapSize = 128;
    defaultRenderConfig.viewportRect.w = cubeMapSize;
    defaultRenderConfig.viewportRect.h = cubeMapSize;
    defaultRenderConfig.postCameraMatrix = ssg.spheres[1].transform;
    defaultRenderConfig.useSceneCamera = false;
    defaultRenderConfig.isCubeMap = true;
    gles30CubeMap.SetRenderConfig(defaultRenderConfig);

    glutSetDefaultErrorMessage();
}

void SSPHH_Application::LoadRenderConfigs()
{
    glutSetErrorMessage(__FILE__, __LINE__, "inside LoadRenderConfigs()");

    hflog.info("%s(): resetting and loading...", __FUNCTION__);
    renderer2.Reset();

    const char* renderconfig_filename = "resources/config/pb_monolithic_2017.renderconfig";
    if (!renderer2.LoadConfig(renderconfig_filename)) {
        hflog.error("%s(): %s file not found.", __FUNCTION__, renderconfig_filename);
    }

    renderer2.LoadShaders();

    defaultRenderConfig.zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");
    defaultRenderConfig.shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");

    rectShadowRenderConfig.zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");
    rectShadowRenderConfig.shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");

    cubeShadowRenderConfig.zShaderProgram = renderer2.FindProgram("cube_shadow", "cube_shadow");

    gles30CubeMap.GetRenderConfig().shaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultSlowProgram");
    gles30CubeMap.GetRenderConfig().zShaderProgram = renderer2.FindProgram("pb_monolithic", "DefaultZProgram");

    glutSetDefaultErrorMessage();
}

void SSPHH_Application::LoadScene()
{
    if (Interface.uf.uf_type == UfType::Broker) {
        hflog.info("configured to be a broker, so not loading scene");
        return;
    }

    FilePathInfo fpi(sceneFilename);
    Interface.sceneName = fpi.fname;
    ssg.Load(sceneFilename);
    ssg.BuildBuffers();

    gles30CubeMap.SetSceneGraph(ssg);
}

void SSPHH_Application::OptimizeClippingPlanes()
{
    //Matrix4f cameraMatrix = defaultRenderConfig.preCameraMatrix * ssg.camera.actualViewMatrix * defaultRenderConfig.postCameraMatrix;
    //cameraMatrix.AsInverse().col4()
    Matrix4f cameraMatrix = ssg.camera.actualViewMatrix.AsInverse();
    const BoundingBoxf& bbox = ssg.GetBoundingBox();
    const Matrix4f& frameOfReference = cameraMatrix;
    const Vector3f& position = frameOfReference.col4().xyz();
    float znear;
    float zfar;

    float distanceToBoxCenter = (position - bbox.Center()).length() + 1.0f;
    float boxRadius = bbox.RadiusRounded();
    znear = max(0.1f, distanceToBoxCenter - boxRadius);
    zfar = distanceToBoxCenter + 2 * boxRadius; // min(1000.0f, distanceToBoxCenter + boxRadius);

    defaultRenderConfig.znear = znear;
    defaultRenderConfig.zfar = zfar;
    rectShadowRenderConfig.znear = max(0.1f, ssg.environment.sunShadowMapNearZ);
    rectShadowRenderConfig.zfar = min(1000.0f, ssg.environment.sunShadowMapFarZ);
}

void SSPHH_Application::ReloadRenderConfigs()
{
    Viperfish::StopWatch stopwatch;
    InitRenderConfigs();
    LoadRenderConfigs();
    stopwatch.Stop();
    hflog.info("%s(): reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
    Interface.lastRenderConfigLoadTime = stopwatch.GetMillisecondsElapsed();
}

void SSPHH_Application::ReloadScenegraph()
{
    ssg.Reset();
    Viperfish::StopWatch stopwatch;
    LoadScene();
    stopwatch.Stop();
    hflog.info("%s(): SSG reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
    Interface.lastScenegraphLoadTime = stopwatch.GetMillisecondsElapsed();
}

void SSPHH_Application::ParseCommandArguments(const vector<string>& args)
{
    if (args.size() <= 1)
        return;

    size_t count = args.size();
    int i = 1;
    for (int j = 1; j < count; j++) {
        bool nextArgExists = j < count - 1;
        if ((args[j] == "-scene") && nextArgExists) {
            // next argument is the path
            FilePathInfo fpi(args[j + 1]);
            if (fpi.Exists()) {
                sceneFilename = args[j + 1];
                hflog.info("%s(): loading scene file %s", __FUNCTION__, sceneFilename.c_str());
            } else {
                hflog.error("%s(): scene file %s does not exist.", __FUNCTION__, sceneFilename.c_str());
            }
            j++;
        }

        if (args[j] == "-broker") {
            Interface.uf.uf_type = UfType::Broker;
            hflog.info("Unicornfish: starting in broker mode");
        }
        if (args[j] == "-worker") {
            Interface.uf.uf_type = UfType::Worker;
            hflog.info("Unicornfish: starting in client mode");
        }
        if (args[j] == "-client") {
            Interface.uf.uf_type = UfType::Client;
            hflog.info("Unicornfish: starting in worker mode");
        }
        if ((args[j] == "-endpoint") && nextArgExists) {
            Interface.uf.endpoint = args[j + 1];
            j++;
            hflog.info("Unicornfish: using endpoint %s", Interface.uf.endpoint.c_str());
        }
        if ((args[j] == "-service") && nextArgExists) {
            Interface.uf.service = args[j + 1];
            hflog.info("Unicornfish: using service %s", Interface.uf.service.c_str());
            j++;
        }
    }
}

void SSPHH_Application::InitUnicornfish()
{
    if (Interface.uf.uf_type != UfType::None) {
        Interface.tools.showMaterialEditor = false;
        Interface.tools.showSphlEditor = false;
        Interface.tools.showScenegraphEditor = false;
        Interface.tools.showDebugView = false;
        Interface.tools.showSSPHHWindow = false;
        Interface.tools.showRenderConfigWindow = false;
        Interface.tools.showUnicornfishWindow = true;
    }
}

void SSPHH_Application::KillUnicornfish()
{
    InterfaceInfo::UNICORNFISHWINDOW& uf = Interface.uf;

    ssphhUf.Stop();
    ssphhUf.Join();
}

void SSPHH_Application::StartPython()
{
    //python.init(string("ssphh"));
    //python.start();
    //python.run();
}

int init_count = 0;
void SSPHH_Application::OnInit(const vector<string>& args)
{
    init_count++;
    // TODO: I would like to make the following code work:
    //
    // SceneGraph sg;
    // sg.Init();
    // sg.Load("myscene.scn");
    //
    // Renderer r;
    // r.Init();
    // r.LoadRenderConfig("deferred.renderconfig")
    // r.LoadRenderConfig("pbr_monolithic.renderconfig")
    // r.LoadRenderConfig("pbr_deferred.renderconfig")
    // r.BuildBuffers(sg);

    ParseCommandArguments(args);

    InitUnicornfish();

    Viperfish::StopWatch stopwatch;

    // StartPython();

    InitRenderConfigs();
    LoadRenderConfigs();

    glutSetErrorMessage(__FILE__, __LINE__, "inside OnInit()");

    Interface.preCameraMatrix.LoadIdentity();

    my_hud_info.glRendererString = string((char*)glGetString(GL_RENDERER));
    my_hud_info.glVendorString = string((char*)glGetString(GL_VENDOR));
    my_hud_info.glVersionString = string((char*)glGetString(GL_VERSION));

    if (!enviroCubeTexture3.LoadTextureCoronaCubeMap("export_cubemap.png", true)) {
        hflog.error("%s(): enviroCubeTexture3...Could not load export_cubemap.png", __FUNCTION__);
    } else {
        hflog.info("Loaded enviroCubeTexture3...loaded export_cubemap.png");
    }

    PBSkyCubeMap.Create();
    PBSkyCubeMap.SetTextureCubeMap(GL_RGB, GL_FLOAT, 64, 64, nullptr, true);
    PBSkyCubeMap.samplerObject.Create();
    PBSkyCubeMap.samplerObject.SetMinFilter(GL_LINEAR);
    PBSkyCubeMap.samplerObject.SetMagFilter(GL_LINEAR);
    PBSkyCubeMap.samplerObject.SetWrapSTR(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    // Initialize default sampler objects
    defaultCubeTextureSampler.Create();
    defaultCubeTextureSampler.SetMagFilter(GL_LINEAR);
    defaultCubeTextureSampler.SetMinFilter(GL_LINEAR);
    defaultCubeTextureSampler.SetWrapSTR(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    default2DTextureSampler.Create();
    default2DTextureSampler.SetMagFilter(GL_LINEAR);
    default2DTextureSampler.SetMinFilter(GL_LINEAR);
    default2DTextureSampler.SetWrapST(GL_REPEAT, GL_REPEAT);
    defaultShadowCubeTextureSampler.Create();
    defaultShadowCubeTextureSampler.SetMagFilter(GL_LINEAR);
    defaultShadowCubeTextureSampler.SetMinFilter(GL_LINEAR);
    //defaultShadowCubeTextureSampler.SetCompareFunction(GL_LESS);
    //defaultShadowCubeTextureSampler.SetCompareMode(GL_COMPARE_REF_TO_TEXTURE);
    defaultShadow2DTextureSampler.Create();
    defaultShadow2DTextureSampler.SetMagFilter(GL_LINEAR);
    defaultShadow2DTextureSampler.SetMinFilter(GL_LINEAR);
    //defaultShadow2DTextureSampler.SetCompareFunction(GL_LESS);
    //defaultShadow2DTextureSampler.SetCompareMode(GL_COMPARE_REF_TO_TEXTURE);

    glutSetErrorMessage(__FILE__, __LINE__, "before loading scene");

    LoadScene();

    glutSetDefaultErrorMessage();

    ResetScene();

    stopwatch.Stop();
    hflog.info("%s(): took %3.2f seconds", __FUNCTION__, stopwatch.GetSecondsElapsed());
}

void SSPHH_Application::OnKill()
{
    // TODO: I would like to make the following code work:
    //
    // SceneGraph sg;
    // Renderer r;
    // ...
    // sg.Kill();
    // r.Kill();
    // r.Init();

    renderer.Reset();
    KillUnicornfish();
    //python.kill();
    //python.join();
}

//void SSPHH_Application::OnKeyDown(int key)
//{
//	return;
//	int modifiers = glutGetModifiers();
//	if (modifiers == 0)
//	{
//		if (key == 27)
//		{
//			glutLeaveMainLoop();
//		}
//
//		if (key == '1')
//		{
//			Viperfish::StopWatch stopwatch;
//			InitRenderConfigs();
//			LoadRenderConfigs();
//			stopwatch.Stop();
//			hflog.info("%s(): reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
//		}
//
//		if (key == '2')
//		{
//			Viperfish::StopWatch stopwatch;
//			ssg.Load(sceneFilename);
//			stopwatch.Stop();
//			hflog.info("%s(): SSG reload took %4.2f milliseconds", __FUNCTION__, stopwatch.GetMillisecondsElapsed());
//		}
//
//		if (tolower(key) == '3') {
//			counter++;
//			if (counter > 3)
//				counter = 0;
//		}
//
//		if (key == '4')
//		{
//			python.addCommand(KASL::PythonInterpreter::Command::PyRunFile, "resources/scripts/guitest.py");
//		}
//
//		if (key == '5')
//		{
//			python.addCommand(KASL::PythonInterpreter::Command::PyRunString, "print('hello')");
//		}
//
//		if (key == '6')
//		{
//			Interface.drawSkyBox = !Interface.drawSkyBox;
//		}
//
//		if (key == '7')
//		{
//
//		}
//
//		if (key == '8')
//		{
//
//		}
//
//		if (key == '9')
//		{
//
//		}
//
//		if (key == 'r' || key == '\t')
//		{
//			Interface.cameraMatrix.LoadIdentity();
//			ResetScene();
//		}
//
//		if (key == ' ')
//		{
//			Interface.enableOrbit = !Interface.enableOrbit;
//		}
//
//		if (key == '-' || key == '_')
//		{
//			// decrease 1 hour
//			AdvanceSunClock(-3600.0, true);
//		}
//
//		if (key == '=' || key == '+')
//		{
//			// increase 1 hour
//			AdvanceSunClock(3600.0, true);
//		}
//
//		if (tolower(key) == 'a') Interface.moveLeft = true;
//		if (tolower(key) == 'd') Interface.moveRight = true;
//		if (tolower(key) == 'w') Interface.moveForward = true;
//		if (tolower(key) == 's') Interface.moveBackward = true;
//		if (tolower(key) == 'z') Interface.moveUp = true;
//		if (tolower(key) == 'c') Interface.moveDown = true;
//		if (tolower(key) == 'q') Interface.rollLeft = true;
//		if (tolower(key) == 'e') Interface.rollRight = true;
//	}
//	else if (modifiers &= GLUT_ACTIVE_SHIFT)
//	{
//		if (tolower(key) == 'h') Interface.decreaseLongitude = true;
//		if (tolower(key) == 'j') Interface.increaseLongitude = true;
//		if (tolower(key) == 'k') Interface.decreaseLatitude = true;
//		if (tolower(key) == 'l') Interface.increaseLatitude = true;
//	}
//	else if (modifiers &= GLUT_ACTIVE_ALT)
//	{
//
//	}
//}
//
//
//void SSPHH_Application::OnKeyUp(int key)
//{
//	if (tolower(key) == 'a') Interface.moveLeft = false;
//	if (tolower(key) == 'd') Interface.moveRight = false;
//	if (tolower(key) == 'w') Interface.moveForward = false;
//	if (tolower(key) == 's') Interface.moveBackward = false;
//	if (tolower(key) == 'z') Interface.moveUp = false;
//	if (tolower(key) == 'c') Interface.moveDown = false;
//	if (tolower(key) == 'q') Interface.rollLeft = false;
//	if (tolower(key) == 'e') Interface.rollRight = false;
//}
//
//
//void SSPHH_Application::OnSpecialKeyDown(int key)
//{
//	return;
//
//	int modifiers = glutGetModifiers();
//
//	if (key == GLUT_KEY_LEFT) Interface.yawLeft = true;
//	if (key == GLUT_KEY_RIGHT) Interface.yawRight = true;
//	if (key == GLUT_KEY_UP) Interface.pitchUp = true;
//	if (key == GLUT_KEY_DOWN) Interface.pitchDown = true;
//
//	if (key == GLUT_KEY_F1) Interface.showHelp = !Interface.showHelp;
//	if (key == GLUT_KEY_F2) Interface.showMainHUD = !Interface.showMainHUD;
//	if (key == GLUT_KEY_F3) Interface.showHUD = !Interface.showHUD;
//	if (key == GLUT_KEY_F4) Interface.recomputeSky = !Interface.recomputeSky;
//	if (key == GLUT_KEY_F5 && modifiers == 0) Interface.saveCoronaSCN = true;
//	if (key == GLUT_KEY_F6 && modifiers == 0) Interface.saveCoronaCubeMapSCN = true;
//	if (key == GLUT_KEY_F5 && modifiers == GLUT_ACTIVE_CTRL) Interface.renderCoronaSCN = true;
//	if (key == GLUT_KEY_F6 && modifiers == GLUT_ACTIVE_CTRL) Interface.renderCoronaCubeMapSCN = true;
//
//	if (key == GLUT_KEY_F11)
//	{
//		SavePbskyTextures();
//	}
//
//	if (key == GLUT_KEY_F12) {
//		renderMode++;
//		if (renderMode >= MAX_RENDER_MODES)
//			renderMode = 0;
//	}
//}
//
//
//void SSPHH_Application::OnSpecialKeyUp(int key)
//{
//	return;
//
//	if (key == GLUT_KEY_LEFT) Interface.yawLeft = false;
//	if (key == GLUT_KEY_RIGHT) Interface.yawRight = false;
//	if (key == GLUT_KEY_UP) Interface.pitchUp = false;
//	if (key == GLUT_KEY_DOWN) Interface.pitchDown = false;
//}

void SSPHH_Application::OnKeyDown(const string& key, int keymod)
{
    Widget::OnKeyDown(key, keymod);

    if (keymod == keyboard.CtrlKeyBit) {
        if (key == "F5")
            Interface.renderCoronaSCN = true;
        if (key == "F6")
            Interface.renderCoronaCubeMapSCN = true;
    }

    if (keymod == keyboard.ShiftKeyBit) {
    }

    if (keymod == 0) {
        if (Interface.showImGui == false && key == "Escape") {
            glutLeaveMainLoop();
        } else if (Interface.showImGui == true && key == "Escape") {
            return;
        }

        if (key == "1") {
            ReloadRenderConfigs();
        }

        if (key == "2") {
            ReloadScenegraph();
        }

        if (key == "3") {
            counter++;
            if (counter > 3)
                counter = -1;
        }

        if (key == "4") {
            Interface.tools.shaderDebugChoice = clamp(Interface.tools.shaderDebugChoice - 1, 0, 20);
            // python.addCommand(KASL::PythonInterpreter::Command::PyRunFile, "resources/scripts/guitest.py");
        }

        if (key == "5") {
            Interface.tools.shaderDebugChoice = clamp(Interface.tools.shaderDebugChoice + 1, 0, 20);
            // python.addCommand(KASL::PythonInterpreter::Command::PyRunString, "print('hello')");
        }

        if (key == "6") {
            if (Interface.tools.shaderDebugSphl != SphlSunIndex)
                Interface.tools.shaderDebugSphl = SphlSunIndex;
            else
                Interface.tools.shaderDebugSphl = -1;
        }

        if (key == "7") {
            Interface.tools.shaderDebugSphl = clamp(Interface.tools.shaderDebugSphl - 1, -1, SphlSunIndex);
        }

        if (key == "8") {
            Interface.tools.shaderDebugSphl = clamp(Interface.tools.shaderDebugSphl + 1, -1, SphlSunIndex);
        }

        if (key == "9") {
            Interface.drawSkyBox = !Interface.drawSkyBox;
        }

        if (key == "r" || key == "Tab") {
            Interface.cameraOrbit.reset();
            Interface.preCameraMatrix.LoadIdentity();
            ResetScene();
        }

        if (key == "n" || key == "N") {
            UseCurrentTime();
        }

        if (key == " " || key == "o" || key == "O") {
            Interface.enableOrbit = !Interface.enableOrbit;
        }

        if (key == "i" || key == "I") {
            imguiCoronaGenerateSphlINIT();
            imguiCoronaGenerateSky();
        }

        if (key == "v" || key == "V") {
            imguiCoronaGenerateSphlVIZ();
        }

        if (key == "g" || key == "G") {
            imguiCoronaGenerateSphlGEN();
        }

        if (key == "h" || key == "H") {
            imguiCoronaGenerateSphlHIER();
        }

        if (key == "j")
            Interface.cameraOrbit.x -= 5.0f;
        if (key == "k")
            Interface.cameraOrbit.x += 5.0f;

        if (key == "[")
            ssg.environment.toneMapExposure = clamp(ssg.environment.toneMapExposure - 0.5f, -12.0f, 12.0f);
        if (key == "]")
            ssg.environment.toneMapExposure = clamp(ssg.environment.toneMapExposure + 0.5f, -12.0f, 12.0f);
        if (key == ";")
            Interface.tools.shaderDebugChoice = clamp(Interface.tools.shaderDebugChoice - 1, 0, 20);
        if (key == "'")
            Interface.tools.shaderDebugChoice = clamp(Interface.tools.shaderDebugChoice + 1, 0, 20);
        if (key == ",")
            Interface.ssphh.MaxDegrees = clamp(Interface.ssphh.MaxDegrees - 1, 0, 9);
        if (key == ".")
            Interface.ssphh.MaxDegrees = clamp(Interface.ssphh.MaxDegrees + 1, 0, 9);

        if (key == "," || key == ".")
            imguiCoronaGenerateSphlHIER();

        if (key == "-" || key == "_") {
            // decrease 1 hour
            AdvanceSunClock(-3600.0, true);
        }

        if (key == "=" || key == "+") {
            // increase 1 hour
            AdvanceSunClock(3600.0, true);
        }

        if (key == "F1")
            Interface.showHelp = !Interface.showHelp;
        if (key == "F2")
            Interface.showMainHUD = !Interface.showMainHUD;
        if (key == "F3")
            Interface.showHUD = !Interface.showHUD;
        if (key == "F4")
            Interface.showDeferredHUD = !Interface.showDeferredHUD;
        if (key == "F5")
            Interface.recomputeSky = !Interface.recomputeSky;
        if (key == "F6")
            imguiCoronaGenerateSky();
        if (key == "F7")
            Interface.ssphh.enableShowSPHLs = !Interface.ssphh.enableShowSPHLs;
        if (key == "F8")
            Interface.showImGui = !Interface.showImGui;
        if (key == "F10")
            Interface.showImGui = !Interface.showImGui;

        if (key == "F11") {
            SavePbskyTextures();
        }

        if (key == "F12") {
            renderMode++;
            if (renderMode >= MAX_RENDER_MODES)
                renderMode = 0;
        }
    }
}

void SSPHH_Application::OnKeyUp(const string& key, int keymod)
{
    Widget::OnKeyUp(key, keymod);
}

void SSPHH_Application::OnMouseMove(int x, int y)
{
    if (Interface.showImGui)
        return;

    Widget::OnMouseMove(x, y);
}

void SSPHH_Application::OnMouseButtonDown(int button)
{
    Widget::OnMouseButtonDown(button);
}

void SSPHH_Application::OnMouseButtonUp(int button)
{
    Widget::OnMouseButtonUp(button);
}

void SSPHH_Application::OnMouseClick(int button, const MouseClickState& mcs)
{
    Widget::OnMouseClick(button, mcs);
}

void SSPHH_Application::OnMouseDoubleClick(int button, const MouseDoubleClickState& mdcs)
{
    Widget::OnMouseDoubleClick(button, mdcs);
}

void SSPHH_Application::OnMouseDrag(int button, const MouseDragState& mds)
{
    Widget::OnMouseDrag(button, mds);
    if (button == 2) {
        renderer2.SetDeferredSplit(mds.currentPosition);
    }
}

void SSPHH_Application::AdvanceSunClock(double numSeconds, bool recomputeSky)
{
    pbsky_timeOffsetInSeconds += numSeconds;
    ssg.environment.pbsky.SetTime(pbsky_localtime, (float)pbsky_timeOffsetInSeconds);
    Interface.recomputeSky = true;
}

void SSPHH_Application::OnUpdate(double deltaTime)
{
    Widget::OnUpdate(deltaTime);

    Viperfish::StopWatch stopwatch;
    DoInterfaceUpdate(deltaTime);

    if (Interface.enableSunCycle) {
        // one day in five minutes
        AdvanceSunClock(deltaTime * 288.0);
    }

    //pbskyAge += deltaTime;
    //if (pbskyAge > pbskyMaxAge) {
    //	Interface.recomputeSky = true;
    //	pbskyAge = 0.0;
    //}

    if (Interface.recomputeSky) {
        ssg.environment.Update(ssg.GetBoundingBox());
        ssg.environment.ComputePBSky();
        Interface.recomputeSky = false;
    }

    if (Interface.enableOrbit) {
        rotX += (float)deltaTime;
        rotY += (float)deltaTime;
        rotZ += (float)deltaTime;

        Interface.cameraOrbit.x += (float)deltaTime;
        Interface.cameraOrbit.y = 0.5f * (float)deltaTime * (Interface.cameraOrbit.y); // +(float)sin(0.25 * rotY) * 15.0f - 15.0f);

        //	for (auto geoIt = ssg.geometry.begin(); geoIt != ssg.geometry.end(); geoIt++)
        //	{
        //		if (geoIt->second.objectName == "teapot")
        //		{
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 1.0, 0.0);
        //		}
        //		if (geoIt->second.objectName == "torusknot")
        //		{
        //			geoIt->second.addlTransform.Rotate(deltaTime, 1.0, 0.0, 0.0);
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 1.0, 0.0);
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 0.0, 1.0);
        //		}
        //		if (geoIt->second.objectName == "bunny")
        //		{
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 1.0, 0.0);
        //		}
        //		if (geoIt->second.objectName == "dragon")
        //		{
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 1.0, 0.0);
        //		}
        //		if (geoIt->second.objectName == "dragon2")
        //		{
        //			geoIt->second.addlTransform.Rotate(deltaTime, 0.0, 1.0, 0.0);
        //		}
        //	}
    }

    ssg.environment.pbskyColorMapSamplerId = defaultCubeTextureSampler.GetId();
    ssg.environment.enviroColorMapSamplerId = defaultCubeTextureSampler.GetId();
    ssg.environment.sunColorMapSamplerId = defaultCubeTextureSampler.GetId();
    ssg.environment.sunDepthMapSamplerId = defaultCubeTextureSampler.GetId();

    stopwatch.Stop();
    my_hud_info.onUpdateTime = stopwatch.GetMillisecondsElapsed();
    my_hud_info.pbskyTime = ssg.environment.LastSkyGenTime();

    if (Interface.saveCoronaSCN) {
        Interface.saveCoronaSCN = false;
        coronaScene.WriteSCN("output.scn", ssg);
    }

    if (Interface.saveCoronaCubeMapSCN) {
        Interface.saveCoronaCubeMapSCN = false;
        Vector3f cameraPosition = ssg.camera.viewMatrix.col4().xyz();
        coronaScene.WriteCubeMapSCN("output_cubemap.scn", ssg, cameraPosition);
    }

    if (Interface.renderCoronaCubeMapSCN) {
        Interface.renderCoronaCubeMapSCN = false;
    }

    GI_ScatterJobs();
    GI_GatherJobs();
}

void SSPHH_Application::DoInterfaceUpdate(double deltaTime)
{
    double moveX = 0.0;
    double moveY = 0.0;
    double moveZ = 0.0;
    double turnX = 0.0;
    double turnY = 0.0;
    double turnZ = 0.0;
    double gpMoveRate = 1.0;
    double gpTurnRate = 1.0;

    if (!Interface.showImGui) //gamepads[0].IsConnected())
    {
        Interface.moveLeft = gamepads[0].lthumbLeft() || kbgamepad.lthumbLeft();
        Interface.moveRight = gamepads[0].lthumbRight() || kbgamepad.lthumbRight();
        Interface.moveForward = gamepads[0].lthumbUp() || kbgamepad.lthumbUp();
        Interface.moveBackward = gamepads[0].lthumbDown() || kbgamepad.lthumbDown();
        Interface.moveUp = gamepads[0].aPressed() || kbgamepad.aPressed();
        Interface.moveDown = gamepads[0].bPressed() || kbgamepad.bPressed();
        Interface.pitchUp = gamepads[0].rthumbUp() || kbgamepad.rthumbUp();
        Interface.pitchDown = gamepads[0].rthumbDown() || kbgamepad.rthumbDown();
        Interface.yawLeft = gamepads[0].rthumbLeft() || kbgamepad.rthumbLeft();
        Interface.yawRight = gamepads[0].rthumbRight() || kbgamepad.rthumbRight();
        Interface.rollLeft = gamepads[0].r2Pressed() || kbgamepad.r2Pressed();
        Interface.rollRight = gamepads[0].l2Pressed() || kbgamepad.l2Pressed();

        if (gamepads[0].lthumbAmount() > 0.0f) {
            gpMoveRate = gamepads[0].lthumbAmount();
        } else {
            gpMoveRate = 1.0;
        }
        if (gamepads[0].rthumbAmount() > 0.0f) {
            gpTurnRate = gamepads[0].rthumbAmount();
        } else {
            gpTurnRate = 1.0;
        }
    }

    double mdx = mouse.buttons[0] ? mouse.dragStates[0].currentDelta.x : 0.0;
    double mdy = mouse.buttons[0] ? mouse.dragStates[0].currentDelta.y : 0.0;

    double scale = -180.0 / Vector2f((float)renderer2.GetDeferredRect().w, (float)renderer2.GetDeferredRect().h).length();
    mdx *= scale;
    mdy *= scale;

    Interface.decreaseLongitude = keyboard.IsPressed("H", 0);
    Interface.increaseLongitude = keyboard.IsPressed("J", 0);
    Interface.decreaseLatitude = keyboard.IsPressed("K", 0);
    Interface.increaseLatitude = keyboard.IsPressed("L", 0);

    moveX = gamepads[0].lthumb_x() + kbgamepad.lthumb_x();
    moveZ = -gamepads[0].lthumb_y() - kbgamepad.lthumb_y();
    moveY = gamepads[0].a() - gamepads[0].b() + kbgamepad.a() - kbgamepad.b();
    turnY = -gamepads[0].rthumb_x() - kbgamepad.rthumb_x() + mdx;
    turnX = gamepads[0].rthumb_y() + kbgamepad.rthumb_y() + mdy;
    turnZ = gamepads[0].r2() - gamepads[0].l2() - kbgamepad.r2() + kbgamepad.l2();

    //if (Interface.moveLeft) moveX -= 1.0;
    //if (Interface.moveRight) moveX += 1.0;
    //if (Interface.moveUp) moveY += 1.0;
    //if (Interface.moveDown) moveY -= 1.0;
    //if (Interface.moveForward) moveZ -= 1.0;
    //if (Interface.moveBackward) moveZ += 1.0;
    //if (Interface.yawLeft) turnY += 1.0;
    //if (Interface.yawRight) turnY -= 1.0;
    //if (Interface.rollLeft) turnZ += 1.0;
    //if (Interface.rollRight) turnZ -= 1.0;
    //if (Interface.pitchUp) turnX += 1.0;
    //if (Interface.pitchDown) turnX -= 1.0;

    if (moveX == 0)
        Interface.moveX = 0;
    if (moveY == 0)
        Interface.moveY = 0;
    if (moveZ == 0)
        Interface.moveZ = 0;
    if (turnX == 0)
        Interface.turnX = 0;
    if (turnY == 0)
        Interface.turnY = 0;
    if (turnZ == 0)
        Interface.turnZ = 0;

    Interface.moveX += (moveX - Interface.moveX) * Interface.moveRate * gpMoveRate * deltaTime;
    Interface.moveY += (moveY - Interface.moveY) * Interface.moveRate * gpMoveRate * deltaTime;
    Interface.moveZ += (moveZ - Interface.moveZ) * Interface.moveRate * gpMoveRate * deltaTime;
    Interface.turnX += (turnX - Interface.turnX) * Interface.turnRate * gpTurnRate * deltaTime;
    Interface.turnY += (turnY - Interface.turnY) * Interface.turnRate * gpTurnRate * deltaTime;
    Interface.turnZ += (turnZ - Interface.turnZ) * Interface.turnRate * gpTurnRate * deltaTime;

    Matrix4f rot;
    rot.Rotate(Interface.turnX, 1.0, 0.0, 0.0);
    rot.Rotate(Interface.turnY, 0.0, 1.0, 0.0);
    rot.Rotate(Interface.turnZ, 0.0, 0.0, 1.0);

    Matrix4f trans;
    trans.Translate(Interface.moveX, Interface.moveY, Interface.moveZ);

    Interface.preCameraMatrix.MultMatrix(rot);
    Interface.preCameraMatrix.MultMatrix(trans);
    Interface.inversePreCameraMatrix = Interface.preCameraMatrix.AsInverse();

    ssg.camera.fov = Interface.ssg.cameraFOV;

    Interface.postCameraMatrix.LoadIdentity();
    //Interface.postCameraMatrix.Translate(0.0f, 0.0f, Interface.cameraOrbit.z);
    //Interface.postCameraMatrix.Rotate(Interface.cameraOrbit.y, 1.0f, 0.0f, 0.0f);
    //Interface.postCameraMatrix.Rotate(Interface.cameraOrbit.X, 0.0f, 1.0f, 0.0f);
    Interface.postCameraMatrix.Rotate(Interface.cameraOrbit.x, 0.0f, 1.0f, 0.0f);
    Interface.postCameraMatrix.Rotate(Interface.cameraOrbit.y, 1.0f, 0.0f, 0.0f);
    Interface.postCameraMatrix.Translate(0.0f, 0.0f, Interface.cameraOrbit.z);
    Interface.inversePostCameraMatrix = Interface.postCameraMatrix.AsInverse();

    Interface.finalCameraMatrix = Interface.preCameraMatrix * ssg.camera.viewMatrix * Interface.postCameraMatrix;
    Interface.inverseFinalCameraMatrix = Interface.finalCameraMatrix.AsInverse();
    Interface.cameraPosition = Interface.finalCameraMatrix.col4();

    float pbskyLatitude = ssg.environment.pbsky.GetLatitude();
    float pbskyLongitude = ssg.environment.pbsky.GetLongitude();

    if (Interface.increaseLatitude) {
        pbskyLatitude += 5.0f * (float)deltaTime;
    }
    if (Interface.increaseLongitude) {
        pbskyLongitude += 5.0f * (float)deltaTime;
    }
    if (Interface.decreaseLatitude) {
        pbskyLatitude -= 5.0f * (float)deltaTime;
    }
    if (Interface.decreaseLongitude) {
        pbskyLongitude -= 5.0f * (float)deltaTime;
    }

    if (Interface.increaseLatitude || Interface.decreaseLatitude || Interface.increaseLongitude || Interface.decreaseLongitude) {
        ssg.environment.pbsky.SetLocation(pbskyLatitude, pbskyLongitude);
        AdvanceSunClock(0.0, true);
    }
}

const Matrix4f& SSPHH_Application::GetCameraMatrix() const
{
    return Interface.preCameraMatrix;
}

void SSPHH_Application::OnRender()
{
    if (Interface.uf.uf_type == UfType::Broker)
        return;

    if (rectShadowRenderConfig.viewportRect.w != Interface.renderconfig.sunShadowMapSize) {
        InitRenderConfigs();
    }
    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    //return;

    // TODO: I would like to make the following code work:
    //
    // SceneGraph sg;
    // Renderer r;
    // ...
    // r.SetRenderConfig("pbr_monolithic");
    // r.UpdateBuffers(sg);
    // r.Render();

    double renderT0 = hflog.getSecondsElapsed();
    if (renderMode == 0) {
        RenderFixedFunctionGL();
    } else if (renderMode == 1) {
        RenderGLES20();
    } else if (renderMode == 2) {
        RenderGLES30();
    }
    my_hud_info.totalRenderTime = hflog.getSecondsElapsed() - renderT0;

    static double t0 = 0.0;
    double t1 = hflog.getMillisecondsElapsed();
    double dt = t1 - t0;
    if (dt < 1000.0)
        hflog.takeStat("frametime", dt);
    t0 = t1;
    SaveScreenshot();
}

void SSPHH_Application::OnRenderOverlay()
{
    bool isCameraViewMatrixVisible = true;

    float xpos = 0.0f;
    float ypos = screenHeight - 15.0f * 15.0f;

    RenderDeferredHUD();

    if (Interface.showMainHUD) {
        RenderMainHUD();
    }

    if (Interface.showHelp) {
        RenderHelp();
    }

    if (Interface.showHUD) {
        RenderHUD();
    }

    if (Interface.showDeferredHUD) {
        Vector2i split = renderer2.GetDeferredSplitPoint();
        Recti rect = renderer2.GetDeferredRect();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.0f, 0.0f, 0.25f);
        renderer2.RenderDeferred(Recti::UpperLeft);
        glColor4f(0.0f, 1.0f, 0.0f, 0.24f);
        renderer2.RenderDeferred(Recti::UpperRight);
        glColor4f(0.0f, 0.0f, 1.0f, 0.25f);
        renderer2.RenderDeferred(Recti::LowerLeft);
        glColor4f(1.0f, 0.0f, 1.0f, 0.25f);
        renderer2.RenderDeferred(Recti::LowerRight);

        glDisable(GL_BLEND);
    }

    if (Interface.showImGui) {
        RenderImGuiHUD();
    }
}

void SSPHH_Application::RenderHUD()
{
    const int maxLines = 25;
    GLfloat xpos = 0.0f;
    GLfloat ypos = screenHeight - maxLines * 15.0f;

    Matrix4f cameraMatrix = Interface.preCameraMatrix * ssg.camera.viewMatrix;

    const char* renderModes[] = {
        "FIXED FUNCTION",
        "GLES20",
        "GLES30",
    };

    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Renderer: %s", my_hud_info.glRendererString.c_str());
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Vendor:   %s", my_hud_info.glVendorString.c_str());
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Version:  %s", my_hud_info.glVersionString.c_str());

    ypos += 15.0f;
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "OnUpdate() time:    % 4.2f", my_hud_info.onUpdateTime);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Total Render time:  % 4.2f", my_hud_info.totalRenderTime);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "G-Buffer pass time: % 4.2f", my_hud_info.gbufferPassTime);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Deferred pass time: % 4.2f", my_hud_info.deferredPassTime);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Pbsky render time:  % 4.2f", my_hud_info.pbskyTime);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Pbsky RGB min/max:  MIN: % 4.2e -- MAX: % 4.2e",
        ssg.environment.pbsky.getMinRgbValue(), ssg.environment.pbsky.getMaxRgbValue());
    ypos += 15.0f;
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "Render mode: %d %s", renderMode, renderModes[renderMode]);
    ypos += 15.0f;
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m11, cameraMatrix.m12, cameraMatrix.m13, cameraMatrix.m14);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m21, cameraMatrix.m22, cameraMatrix.m23, cameraMatrix.m24);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m31, cameraMatrix.m32, cameraMatrix.m33, cameraMatrix.m34);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m41, cameraMatrix.m42, cameraMatrix.m43, cameraMatrix.m44);

    Vector4f eye = cameraMatrix * Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "CAMERA: X: % -3.2f, Y: % -3.2f, Z: % -3.2f", eye.x, eye.y, eye.z);
    Vector3f sun = ssg.environment.pbsky.GetSunVector();
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "SUN ANGLES:   AZ: % -3.1f, ALT: % -3.1f",
        ssg.environment.pbsky.GetSunAzimuth(),
        ssg.environment.pbsky.GetSunAltitude());
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "SUN POSITION: X: % -2.2f  Y: % -2.2f  Z: % -2.2f", sun.x, sun.y, sun.z);
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "MAP POSITION: LAT: % -3.2f   LONG: % -3.2f", ssg.environment.pbsky.GetLatitude(), ssg.environment.pbsky.GetLongitude());

    Color4f sunDiskRadiance = ssg.environment.pbsky.GetSunDiskRadiance();
    Color4f groundRadiance = ssg.environment.pbsky.GetGroundRadiance();
    // Sun Disk Radiance
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "SUN DISK RADIANCE: R: % -3.1f  G: % -3.1f  B: % -3.2f  AVG: % -3.2f",
        sunDiskRadiance.r, sunDiskRadiance.g, sunDiskRadiance.b,
        ssg.environment.pbsky.GetAverageRadiance());

    // Ground Radiance
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "GROUND RADIANCE:   R: % -3.1f  G: % -3.1f  B: % -3.2f",
        groundRadiance.r, groundRadiance.g, groundRadiance.b);

    float seconds = (float)(ssg.environment.pbsky.getSec() + ssg.environment.pbsky.getSecFract());
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, GLUT_BITMAP_9_BY_15, "DATE: %02d/%02d/%02d %02d:%02d:%02d%.3f  LST: %2.3f",
        ssg.environment.pbsky.getMonth(),
        ssg.environment.pbsky.getDay(),
        ssg.environment.pbsky.getYear(),
        ssg.environment.pbsky.getHour(),
        ssg.environment.pbsky.getMin(),
        ssg.environment.pbsky.getSec(),
        ssg.environment.pbsky.getSecFract(),
        ssg.environment.pbsky.getLST());
}

void SSPHH_Application::RenderMainHUD()
{
    extern void PrintBitmapStringJustified(float x, float y, int justification, void* font, const char* format, ...);
    extern void PrintString9x15(float x, float y, int justification, const char* format, ...);

    extern double g_Fps;
    extern double g_distance;

    glColor3f(1, 1, 1);

    float y = 20.0f;
    PrintString9x15(0, y, 1, "SSPHH 2017");
    y += 15.0f;

    PrintString9x15(0, y, 1, "%.3f fps", g_Fps);
    y += 15.0f;

    PrintString9x15(0, y, 1, "counter: %d", counter);
    y += 15.0f;

    if (counter == 0)
        PrintString9x15(0, y, GLUT_JUSTIFICATION::RIGHT, "NONE");
    if (counter == 1)
        PrintString9x15(0, y, GLUT_JUSTIFICATION::RIGHT, "RECT SHADOW");
    if (counter == 2)
        PrintString9x15(0, y, GLUT_JUSTIFICATION::RIGHT, "SPHERE 1 CUBE MAP");
    if (counter == 3)
        PrintString9x15(0, y, GLUT_JUSTIFICATION::RIGHT, "ENVIRO CUBE MAP");
    y += 15.0f;

    PrintString9x15(0, y, 1, "ESC...QUIT");
    y += 15.0f;

    y = 20.0f;
    PrintString9x15(0, y, 0, "Zoom: %3f", g_distance);

    y += 15.0f;
    PrintString9x15(0, y, 0, "mX: %3.3f", Interface.moveX);
    y += 15.0f;
    PrintString9x15(0, y, 0, "mY: %3.3f", Interface.moveY);
    y += 15.0f;
    PrintString9x15(0, y, 0, "mZ: %3.3f", Interface.moveZ);
    y += 15.0f;
    PrintString9x15(0, y, 0, "tX: %3.3f", Interface.turnX);
    y += 15.0f;
    PrintString9x15(0, y, 0, "tY: %3.3f", Interface.turnY);
    y += 15.0f;
    PrintString9x15(0, y, 0, "tZ: %3.3f", Interface.turnZ);
    y += 15.0f;
    PrintString9x15(0, y, 0, "kgp: %s", kbgamepad.GetHexRepresentation().c_str());
    y += 15.0f;
    PrintString9x15(0, y, 0, "gp0: %s", gamepads[0].GetHexRepresentation().c_str());
    y += 15.0f;
    PrintString9x15(0, y, 0, "gp1: %s", gamepads[1].GetHexRepresentation().c_str());
    y += 15.0f;
    PrintString9x15(0, y, 0, "gp2: %s", gamepads[2].GetHexRepresentation().c_str());
    y += 15.0f;
    PrintString9x15(0, y, 0, "gp3: %s", gamepads[3].GetHexRepresentation().c_str());
    y += 15.0f;
    PrintString9x15(0, y, 0, "screen split position: %i, %i", renderer2.GetDeferredSplitPoint().x, renderer2.GetDeferredSplitPoint().y);

    const vector<string>& history = hflog.getHistory();
    for (int i = 0; i < history.size(); i++) {
        string m = history[i];
        istringstream istr(m);
        string line;
        int j = 0;
        while (getline(istr, line)) {
            if (j == 0) {
                y += 15.0f;
                PrintString9x15(0, y, 0, "debug[%02i]: %s", i, line.c_str());
            } else {
                y += 15.0f;
                PrintString9x15(0, y, 0, "debug[%02i]: > %s", i, line.c_str());
            }
            j++;
        }
    }
}

void SSPHH_Application::RenderHelp()
{
    GLfloat xpos = 0.0f;
    GLfloat ypos = 60.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "-- Actions ----------------");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Toggle Help            [F1]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Toggle Main HUD        [F2]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Toggle HUD             [F3]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Toggle Deferred HUD    [F4]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Recompute Sky          [F5]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Output Corona SCN      [F6]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Output Corona Cube SCN [F7]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Save PBSKY PPMs       [F11]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Render Mode           [F12]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Reload Scene            [1]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Reset Scene             [2]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Increase Counter        [3]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Python Test             [4]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Python GUI              [5]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Sky Box                 [6]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "+ 1 Hour              [-/_]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "- 1 Hour              [=/+]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Rotate objects      [SPACE]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "-- Movement ----------");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Turn Up                [UP]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Turn Down            [DOWN]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Turn Left            [LEFT]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Turn Right          [RIGHT]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Forward            [W]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Backward           [S]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Left               [A]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Right              [D]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Roll Left               [Q]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Roll Right              [E]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Down               [Z]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Move Up                 [C]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "Reset camera            [R]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "-- PBSKY -------------");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "In/de-crease lat [CTRL-l/L]");
    ypos += 15.0f;
    glutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, GLUT_BITMAP_9_BY_15, "In/de-crease long [ALT-l/L]");
    ypos += 15.0f;
}

void SSPHH_Application::RenderDeferredHUD()
{
    if (!Interface.showDeferredHUD)
        return;

    Vector2i splitPoint = renderer2.GetDeferredSplitPoint();
    Recti q[4] = {
        renderer2.GetDeferredRect().GetQuadrant(Recti::UpperLeft, splitPoint),
        renderer2.GetDeferredRect().GetQuadrant(Recti::UpperRight, splitPoint),
        renderer2.GetDeferredRect().GetQuadrant(Recti::LowerLeft, splitPoint),
        renderer2.GetDeferredRect().GetQuadrant(Recti::LowerRight, splitPoint)
    };
    const char* names[] = {
        "Upper Left",
        "Upper Right",
        "Lower Left",
        "Lower Right"
    };
    for (int i = 0; i < 4; i++) {
        if (q[i].h < 15)
            break;
        float y = (float)q[i].y + 15.0f;
        float x = (float)q[i].x;
        PrintString9x15(x, y, 0, "%s (%ix%i)", names[i], q[i].w, q[i].h);
    }
}

void SSPHH_Application::OnReshape(int width, int height)
{
    float w = (float)width;
    float h = (float)height;
    screenWidth = w;
    screenHeight = h;
    aspect = w / h;

    renderer2.SetDeferredRect(Recti(0, 0, (int)w, (int)h));

    defaultRenderConfig.projectionMatrix.LoadIdentity();
    defaultRenderConfig.projectionMatrix.Perspective(ssg.camera.fov, ssg.camera.imageAspect, ssg.camera.imageNearZ, ssg.camera.imageFarZ);

    screenOrthoMatrix.LoadIdentity();
    screenOrthoMatrix.Ortho2D(0.0f, screenWidth, screenHeight, 0.0f);

    hflog.setMaxHistory(height / 30);
}

void SSPHH_Application::RenderFixedFunctionGL()
{
    Matrix4f sunMatrix;
    sunMatrix.LoadIdentity();
    sunMatrix.Rotate(90.0f - ssg.environment.pbsky.GetSunAzimuth(), 0.0f, 1.0f, 0.0f);
    sunMatrix.Rotate(ssg.environment.pbsky.GetSunAltitude(), 1.0f, 0.0f, 0.0f);
    Vector4f sunVector;
    sunVector = sunMatrix * Vector4f(0.0f, 0.0f, -1.0f, 0.0f);
    Vector3f v = ssg.environment.pbsky.GetSunVector().norm();
    sunVector = Vector4f(v.x, v.y, v.z, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, sunVector.const_ptr());

    // Set up camera and scene projection

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(ssg.camera.fov, aspect, 0.01, 200.0);
    glMultMatrixf((Interface.preCameraMatrix).AsInverse().const_ptr());
    //glMultMatrixf((ssg.camera.viewMatrix).const_ptr());

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Render Sun and Environment

    // glutEnvironmentCube(50.0f, enviroCubeTexture1.GetTextureId());
    glutEnvironmentCube(50.0f, PBSkyCubeMap.GetTextureId());

    // Render Scene

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    if (ssg.geometryObjects.size() == 0)
        glutSolidTeapot(1.0);

    glDisable(GL_LIGHTING);

    // Camera
    glPushMatrix();
    glMultMatrixf(ssg.camera.viewMatrix.AsInverse().const_ptr());
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.0f);
    glutWireCone(2 * sin(ssg.camera.fov * FX_DEGREES_TO_RADIANS * 0.5), 1.0, 32, 2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.5f);
    glutWireCube(1.0);
    glPopMatrix();
    glutSixAxis(1.0);
    glutWireFrustumf(ssg.camera.projectionMatrix.const_ptr());
    glPopMatrix();

    // Scene center
    glutSixAxis(2.0);

    // Spheres
    for (auto s = ssg.spheres.begin(); s != ssg.spheres.end(); s++) {
        glPushMatrix();
        glMultMatrixf(s->second.transform.const_ptr());
        glutSolidSphere(0.5, 16, 16);
        glPopMatrix();
    }

    // Geometry
    for (auto g = ssg.geometry.begin(); g != ssg.geometry.end(); g++) {
        glPushMatrix();
        glMultMatrixf(g->second.transform.const_ptr());
        glMultMatrixf(g->second.addlTransform.const_ptr());
        Vector3f center = ssg.geometryObjects[g->second.objectId].BoundingBox.Center();

        glPushMatrix();
        glTranslatef(center.x, center.y, center.z);
        glutWireCube(ssg.geometryObjects[g->second.objectId].BoundingBox.MaxSize());
        glPopMatrix();

        glEnable(GL_LIGHTING);
        OBJStaticModel& osm = ssg.geometryObjects[g->second.objectId];
        glEnable(GL_TEXTURE_2D);
        osm.RenderGL11();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glPopMatrix();
    }

    glPushMatrix();
    sunVector *= 20.0f;
    glTranslatef(sunVector.x, sunVector.y, sunVector.z);
    glutSolidSphere(0.5, 16, 16);
    glPopMatrix();

    // Point Lights
    glEnable(GL_BLEND);
    for (auto pl = ssg.pointLights.begin(); pl != ssg.pointLights.end(); pl++) {
        glPushMatrix();
        glTranslatef(pl->position.x, pl->position.y, pl->position.z);
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
        glutSolidSphere(0.5f, 16, 16);
        glColor4f(1.0f, 1.0f, 0.0f, 0.1f);
        glutSolidSphere(pl->falloffRadius, 16, 16);
        glPopMatrix();
    }
    glDisable(GL_BLEND);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void SSPHH_Application::RenderGLES20()
{
    RenderFixedFunctionGL();
    glutSetErrorMessage(__FILE__, __LINE__, "%s: Part 1", __FUNCTION__);
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glutSetDefaultErrorMessage();
}

void SSPHH_Application::DirtySPHLs()
{
    int i = 0;
    for (auto& sphl : ssg.ssphhLights) {
        sphls[i++];
        sphl.dirty = true;
    }
}

void SSPHH_Application::UpdateSPHLs()
{
    if (!coefs_init) {
        geosphere.load("resources/models/icos4.txt");
        //sphl.randomize();
        //sphl.createMesh(geosphere);
        //sphl.createLightProbe();
        sph_renderer.SetSceneGraph(ssg);
        coefs_init = true;
    }

    if (ssg.ssphhLights.size() != sphls.size()) {
        sphls.clear();
        DirtySPHLs();
    }

    // Determine which sphls we need to recalculate and actually draw
    for (auto& sphl : sphls) {
        sphl.second.enabled = false;
    }

    int i = 0;
    for (auto& sphl : ssg.ssphhLights) {
        if (sphl.randomize) {
            //sphls[i].randomize();
            //for (int lm = 0; lm < sphl.GetMaxCoefficients(); lm++)
            //{
            //	sphl.msph[0][lm] = sphls[i].v_coefs[Sphl::RedComponent][lm];
            //	sphl.msph[1][lm] = sphls[i].v_coefs[Sphl::GreenComponent][lm];
            //	sphl.msph[2][lm] = sphls[i].v_coefs[Sphl::BlueComponent][lm];
            //}
            sphl.Randomize();
            sphl.Standardize();
            sphl.randomize = false;
            sphl.dirty = true;
        }
        if (sphl.randomizePosition) {
            sphl.position.reset(randomSampler(-5.0f, 5.0f), randomSampler(2.0f, 3.0f), randomSampler(-6.0f, 6.0f));
            sphl.randomizePosition = false;
            sphl.depthSphlMap.dirty = true;
        }
        if (sphl.dirty) {
            // Copy coefficients from the scene graph SPHL list
            sphls[i].copyCoefficients(sphl, Interface.ssphh.MaxDegrees, Interface.ssphh.enableBasicShowSPHLs);
            sphls[i].createMesh(geosphere);
            //sphls[i].createLightProbe();
            sphl.dirty = false;
            sphl.depthSphlMap.dirty = true;
        }
        sphls[i].position = sphl.position;
        sphls[i].enabled = sphl.enabled;
        i++;
    }
}

void SSPHH_Application::SetupRenderGLES30()
{
    OptimizeClippingPlanes();
    UpdateSPHLs();

    int i = 0;
    for (auto& sphl : ssg.ssphhLights) {
        if (sphl.lightProbe_corona.empty()) {
            sphl.UploadLightProbe(sphl.lightProbe_corona, sphl.coronaLightProbeTexture);
            sphl.UploadLightProbe(sphl.lightProbe_sph, sphl.sphLightProbeTexture);
            sphl.UploadLightProbe(sphl.lightProbe_hier, sphl.hierLightProbeTexture);
        }

        if (Interface.ssphh.enableShadowColorMap) {
            sphls[i].lightProbeTexIds[0] = sphl.colorSphlMap.texture.GetTexture();
        } else {
            sphls[i].lightProbeTexIds[0] = sphl.hierLightProbeTexture.GetTexture();
        }
        sphls[i].lightProbeTexIds[1] = sphl.coronaLightProbeTexture.GetTexture();
        sphls[i].lightProbeTexIds[2] = sphl.sphLightProbeTexture.GetTexture();
        i++;
    }
}

void SSPHH_Application::RenderGLES30()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    SetupRenderGLES30();
    RenderGLES30Shadows();
    RenderGLES30Scene();

    RenderGLES30SPHLs();
    RenderGL11Hierarchies();

    if (counter == 1)
        RenderTest1SunShadows();
    if (counter == 2)
        RenderTest2SphereCubeMap();
    if (counter == 3)
        RenderTest3EnviroCubeMap();

    glutSetDefaultErrorMessage();
}

void SSPHH_Application::RenderGLES30Scene()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    defaultRenderConfig.clearDepthBuffer = true;
    defaultRenderConfig.clearColorBuffer = false;
    defaultRenderConfig.viewportRect.x = 0;
    defaultRenderConfig.viewportRect.y = 0;
    defaultRenderConfig.viewportRect.w = (size_t)screenWidth;
    defaultRenderConfig.viewportRect.h = (size_t)screenHeight;
    defaultRenderConfig.preCameraMatrix = Interface.inversePreCameraMatrix;
    defaultRenderConfig.postCameraMatrix = Interface.inversePostCameraMatrix;
    defaultRenderConfig.fov = Interface.ssg.cameraFOV;
    defaultRenderConfig.isCubeMap = false;
    defaultRenderConfig.useSceneCamera = true;
    defaultRenderConfig.useZOnly = false;
    defaultRenderConfig.useMaterials = true;
    defaultRenderConfig.shaderDebugChoice = Interface.tools.shaderDebugChoice;
    defaultRenderConfig.shaderDebugLight = Interface.tools.shaderDebugLight;
    defaultRenderConfig.shaderDebugSphl = Interface.tools.shaderDebugSphl;

    ssg.AdvancedRender(defaultRenderConfig);
    ssg.camera.actualViewMatrix = defaultRenderConfig.cameraMatrix;

    if (Interface.drawSkyBox) {
        glutSetErrorMessage(__FILE__, __LINE__, "skybox");
        glEnable(GL_DEPTH_TEST);
        RenderSkyBox();
        glDisable(GL_DEPTH_TEST);
        glutSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
    }

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
}

bool SaveTextureMap(GLenum target, GLuint id, const string& path);
void ConvertBuffer(GLenum srccomponents, GLenum srctype, const void* srcdata, GLenum dstcomponents, GLenum dsttype, void* dstdata, GLsizei w, GLsizei h, GLsizei layers = 1);

void ConvertBuffer(GLenum srccomponents, GLenum srctype, const void* srcdata, GLenum dstcomponents, GLenum dsttype, void* dstdata, GLsizei w, GLsizei h, GLsizei layers)
{
    float* srcfpixels = nullptr;
    unsigned char* srcubpixels = nullptr;

    float* dstfpixels = nullptr;
    unsigned char* dstubpixels = nullptr;

    if (srctype == GL_UNSIGNED_BYTE) {
        srcubpixels = (unsigned char*)srcdata;
    } else if (srctype == GL_FLOAT) {
        srcfpixels = (float*)srcdata;
    } else {
        hflog.warning("%s(): Unsupported source format", __FUNCTION__);
    }

    if (dsttype == GL_UNSIGNED_BYTE) {
        dstubpixels = (unsigned char*)dstdata;
    } else if (dsttype == GL_FLOAT) {
        dstfpixels = (float*)dstdata;
    } else {
        hflog.warning("%s(): Unsupported destination format", __FUNCTION__);
    }

    size_t size = w * h * layers;
    size_t addr = 0;
    for (int addr = 0; addr < size; addr++) {
        for (int i = 0; i < (int)dstcomponents; i++) {
            if (i >= (int)srccomponents) {
                if (dstubpixels) {
                    *dstubpixels++ = 0;
                } else if (dstfpixels) {
                    *dstfpixels++ = 0.0f;
                }
            } else {
                if (dstubpixels && srcubpixels) {
                    *dstubpixels++ = *srcubpixels++;
                } else if (dstubpixels && srcfpixels) {
                    float v = *srcfpixels++;
                    *dstubpixels++ = clamp((int)(v * 255.99f), 0, 255);
                } else if (dstfpixels && srcubpixels) {
                    float v = *srcubpixels++;
                    *dstfpixels++ = clamp(v / 255.0f, 0.0f, 1.0f);
                } else if (dstfpixels && srcfpixels) {
                    *dstfpixels++ = *srcfpixels++;
                }
            }
        }
    }
}

bool SaveTextureMap(GLenum target, GLuint id, const string& path)
{
    glActiveTexture(GL_TEXTURE0);
    glutDebugBindTexture(target, id);
    int w = 0;
    int h = 0;
    int internalformat = 0;
    int layers = 1;
    GLenum type = GL_UNSIGNED_BYTE;
    int redSize = 0;
    int greenSize = 0;
    int blueSize = 0;
    int alphaSize = 0;
    int depthSize = 0;
    int redType = 0;
    int greenType = 0;
    int blueType = 0;
    int alphaType = 0;
    int depthType = 0;
    int components = 0;

    if (target == GL_TEXTURE_CUBE_MAP) {
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_RED_SIZE, &redSize);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_RED_TYPE, &redType);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_GREEN_SIZE, &greenSize);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_GREEN_TYPE, &greenType);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_BLUE_SIZE, &blueSize);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_BLUE_TYPE, &blueType);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_ALPHA_SIZE, &alphaSize);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_ALPHA_TYPE, &alphaType);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_DEPTH_SIZE, &depthSize);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_DEPTH_TYPE, &depthType);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_COMPONENTS, &components);
        layers = 6;
    } else if (target == GL_TEXTURE_2D) {
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_RED_SIZE, &redSize);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_RED_TYPE, &redType);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_GREEN_SIZE, &greenSize);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_GREEN_TYPE, &greenType);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_BLUE_SIZE, &blueSize);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_BLUE_TYPE, &blueType);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_ALPHA_SIZE, &alphaSize);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_ALPHA_TYPE, &alphaType);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH_SIZE, &depthSize);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH_TYPE, &depthType);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_COMPONENTS, &components);
    } else {
        return false;
    }

    int stride = w * h;
    int bytesPerPixel = 1;
    if (internalformat == GL_RGB) {
        bytesPerPixel = 3;
        type = GL_UNSIGNED_BYTE;
    } else if (internalformat == GL_RGBA) {
        bytesPerPixel = 4;
        type = GL_UNSIGNED_BYTE;
    } else if (internalformat == GL_RGBA8) {
        bytesPerPixel = 4;
        internalformat = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    } else if (internalformat == GL_DEPTH_COMPONENT32F) {
        bytesPerPixel = 4;
        internalformat = GL_DEPTH_COMPONENT;
        type = GL_FLOAT;
    } else if (internalformat == GL_DEPTH_COMPONENT) {
        bytesPerPixel = 3;
        type = GL_UNSIGNED_BYTE;

    } else {
        hflog.warning("%s(): unknown internal format %04x / %d / %s", __FUNCTION__, internalformat, internalformat, glNameTranslator.GetString(internalformat));
        hflog.warning("%s(): unknown type %d/%s/%s/%s", __FUNCTION__, depthSize, glNameTranslator.GetString(depthType), glNameTranslator.GetString(type), glNameTranslator.GetString(components));
        return false;
    }

    stride *= bytesPerPixel;

    vector<unsigned char> pixels;
    pixels.resize(layers * stride);
    GLenum error = glGetError();
    Image4f i4f;
    Image4ub i4ub;

    if (target == GL_TEXTURE_CUBE_MAP) {
        i4f.resize(w, h, 6);
        for (int face = 0; face < 6; face++) {
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, internalformat, type, &pixels[face * stride]);
            error = glGetError();
            if (error != 0)
                break;
        }
        if (!error) {
            int srccomponents = 0;
            if (internalformat == GL_DEPTH_COMPONENT)
                srccomponents = 1;
            else if (internalformat == GL_RGB)
                srccomponents = 3;
            else if (internalformat == GL_RGBA)
                srccomponents = 4;

            i4f.resize(w, h, 6);
            ConvertBuffer(srccomponents, type, &pixels[0], 4, GL_FLOAT, &i4f.pixels[0], w, h, 6);
            i4f.convertCubeMapToRect();
            i4f.savePPMi(path, 255.99f, 0, 255);
        }
    } else if (target == GL_TEXTURE_2D) {
        glGetTexImage(GL_TEXTURE_2D, 0, internalformat, type, &pixels[0]);
        error = glGetError();
        if (!error) {
            int srccomponents = 0;
            if (internalformat == GL_DEPTH_COMPONENT)
                srccomponents = 1;
            else if (internalformat == GL_RGB)
                srccomponents = 3;
            else if (internalformat == GL_RGBA)
                srccomponents = 4;

            i4f.resize(w, h, 1);
            ConvertBuffer(srccomponents, type, &pixels[0], 4, GL_FLOAT, &i4f.pixels[0], w, h, 1);
            i4f.savePPMi(path, 255.99f, 0, 255);
        }
    }

    if (error != 0) {
        hflog.warning("%s(): glError() -> %s [%s/%s/%s] (R: %d/%s, G: %d/%s, B: %d/%s, A: %d/%s, Z: %d/%s)", __FUNCTION__,
            glNameTranslator.GetString(error),
            glNameTranslator.GetString(internalformat), glNameTranslator.GetString(type), glNameTranslator.GetString(components),
            redSize, glNameTranslator.GetString(redType),
            greenSize, glNameTranslator.GetString(greenType),
            blueSize, glNameTranslator.GetString(blueType),
            alphaSize, glNameTranslator.GetString(alphaType),
            depthSize, glNameTranslator.GetString(depthType));
    }

    bool iszero = true;
    for (auto i = 0; i < pixels.size(); i++) {
        if (pixels[i] != '\0') {
            iszero = false;
            break;
        }
    }

    glutDebugBindTexture(target, 0);
    return true;
}

void SSPHH_Application::RenderGLES30Shadows()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    map<GLenum, RenderTarget*> rts;

    double t0 = hflog.getSecondsElapsed();
    rectShadowRenderConfig.renderSkyBox = false;
    rectShadowRenderConfig.viewportRect.w = Interface.renderconfig.sunShadowMapSize;
    rectShadowRenderConfig.viewportRect.h = Interface.renderconfig.sunShadowMapSize;
    rectShadowRenderConfig.preCameraMatrix.LoadIdentity();
    rectShadowRenderConfig.postCameraMatrix.LoadIdentity();
    rectShadowRenderConfig.projectionMatrix = ssg.environment.sunShadowProjectionMatrix;
    rectShadowRenderConfig.cameraMatrix = ssg.environment.sunShadowViewMatrix;
    rectShadowRenderConfig.cameraPosition = ssg.environment.sunShadowMapOrigin;
    rectShadowRenderConfig.znear = ssg.environment.sunShadowMapNearZ;
    rectShadowRenderConfig.zfar = ssg.environment.sunShadowMapFarZ;
    rectShadowRenderConfig.clearColor.reset(1.0f, 0.0f, 1.0f, 1.0f);
    rectShadowRenderConfig.clearColorBuffer = true;
    rectShadowRenderConfig.renderToFBO = true;
    ssg.AdvancedRender(rectShadowRenderConfig);
    rectShadowRenderConfig.fbo.GenerateMipmaps();

    // Sun Shadow Map is rendered, now let's bind it to a texture_ map...

    for (auto& rt : rectShadowRenderConfig.fbo.renderTargets) {
        if (rt.second.attachment == GL_COLOR_ATTACHMENT0)
            ssg.environment.sunColorMapId = rt.second.object;
        if (rt.second.attachment == GL_DEPTH_ATTACHMENT)
            ssg.environment.sunDepthMapId = rt.second.object;
    }

    //for (int i = 0; i < (int)rectShadowRenderConfig.fbo.renderTargets.size(); i++)
    //{
    //	rts[rectShadowRenderConfig.fbo.renderTargets[i].first] = &(rectShadowRenderConfig.fbo.renderTargets[i].second);
    //}
    //for (auto rtIt = rts.begin(); rtIt != rts.end(); rtIt++)
    //{
    //	RenderTarget &rt = *(rtIt->second);
    //	if (rt.attachment == GL_COLOR_ATTACHMENT0)
    //	{
    //		ssg.environment.sunColorMapId = rt.object;

    //		rt.unit = ssg.environment.sunColorMapUnit;
    //		rt.sampler = ssg.environment.sunColorMapSamplerId;
    //	}
    //	else if (rt.attachment == GL_DEPTH_ATTACHMENT)
    //	{
    //		ssg.environment.sunDepthMapId = rt.object;

    //		rt.unit = ssg.environment.sunDepthMapUnit;
    //		rt.sampler = ssg.environment.sunDepthMapSamplerId;
    //	}
    //	else
    //	{
    //		rt.unit = 0;
    //		rt.sampler = 0;
    //	}

    //	glutBindTextureAndSampler(rt.unit, rt.target, rt.object, rt.sampler);

    //	int w, h;
    //	int mipLevel = 0;
    //	glGetTexLevelParameteriv(rt.target, mipLevel, GL_TEXTURE_WIDTH, &w);
    //	glGetTexLevelParameteriv(rt.target, mipLevel, GL_TEXTURE_HEIGHT, &h);

    //	glutBindTextureAndSampler(rt.unit, rt.target, 0, 0);
    //	rt.unit = 0;
    //}
    ssg.environment.sunShadowMapTime = (float)(1000.0f * (hflog.getSecondsElapsed() - t0));

    if (Interface.captureShadows) {
        SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunColorMapId, "sun_color.ppm");
        SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunDepthMapId, "sun_depth.ppm");
    }

    // Render cube shadow map for light 0
    for (int i = 0; i < ssg.pointLights.size(); i++) {
        auto& spl = ssg.pointLights[i];
        auto& scs = ssg.pointLights[i].scs;

        scs.zfar = cubeShadowRenderConfig.zfar;
        cubeShadowRenderConfig.fbo_gen_color = false;

        cubeShadowRenderConfig.renderToFBO = false;
        cubeShadowRenderConfig.useSceneCamera = false;
        cubeShadowRenderConfig.cameraMatrix.LoadIdentity();
        cubeShadowRenderConfig.cameraPosition = Vector4f(spl.position, 1.0f);

        RenderCubeShadowMap(ssg, scs, cubeShadowRenderConfig);
        glutSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
    }

    for (int i = 0; i < ssg.ssphhLights.size(); i++) {
        auto& sphl = ssg.ssphhLights[i];
        auto& scs = sphl.depthSphlMap;

        scs.zfar = cubeShadowRenderConfig.zfar;
        if (Interface.ssphh.enableShadowColorMap) {
            sphl.colorSphlMap.texture.CreateTextureCube();
            cubeShadowRenderConfig.clearColor.reset(0.2f, 0.4f, 0.6f, 1.0f);
            cubeShadowRenderConfig.fbo_gen_color = true;
            cubeShadowRenderConfig.fbo_color_map = sphl.colorSphlMap.texture.GetTexture();
        } else {
            cubeShadowRenderConfig.fbo_gen_color = false;
            cubeShadowRenderConfig.fbo_color_map = 0;
        }
        cubeShadowRenderConfig.renderToFBO = false;
        cubeShadowRenderConfig.useSceneCamera = false;
        cubeShadowRenderConfig.cameraMatrix.LoadIdentity();
        cubeShadowRenderConfig.cameraPosition = sphl.position;

        RenderCubeShadowMap(ssg, sphl.depthSphlMap, cubeShadowRenderConfig);
        glutSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);

        if (Interface.captureShadows) {
            ostringstream ostr;
            ostr << "sphl" << setw(2) << setfill('0') << i;
            SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.colorSphlMap.texture.GetTexture(), ostr.str() + "_color.ppm");
            SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.depthSphlMap.texture.GetTexture(), ostr.str() + "_depth.ppm");
        }
    }

    Interface.captureShadows = false;
}

void SSPHH_Application::RenderGLES30SPHLs()
{
    if (!Interface.ssphh.enableShowSPHLs)
        return;
    //Renderer gl;
    SimpleRenderConfiguration rc = defaultRenderConfig;
    rc.shaderProgram = renderer2.FindProgram("sphl", "sphl");
    rc.clearDepthBuffer = false;
    rc.clearColorBuffer = false;
    rc.enableBlend = false;
    rc.blendFuncSrcFactor = GL_SRC_ALPHA;
    rc.blendFuncDstFactor = GL_ONE;

    if (!rc.shaderProgram) {
        hflog.info("%s(): sphl shader not found", __FUNCTION__);
        return;
    }

    GLint vloc = rc.shaderProgram->GetAttribLocation("aPosition");
    GLint tloc = rc.shaderProgram->GetAttribLocation("aTexCoord");

    // BEGIN RENDER SPH
    sph_renderer.SetRenderConfig(rc);
    //auto program1 = rc.shaderProgram = renderer2.FindProgram("sphl", "sphl");
    //auto program2 = rc.shaderProgram = renderer2.FindProgram("glut", "cubemap");
    sph_renderer.SaveGLState();
    if (sph_renderer.ApplyRenderConfig()) {
        glActiveTexture(GL_TEXTURE0);
        for (auto& it : sphls) {
            auto& sphl = it.second;

            if (!sphl.enabled)
                continue;

            // glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[0]);

            rc.shaderProgram->ApplyUniform("SPHL_LightProbeMode", (SimpleUniform)0);
            rc.shaderProgram->ApplyUniform("SPHL_NumDegrees", (SimpleUniform)2);
            rc.shaderProgram->ApplyUniform("SPHL_Coefs", SimpleUniform(GL_FLOAT_VEC4, 9, GL_FALSE, (GLubyte*)sphl.coefs));
            rc.shaderProgram->ApplyUniform("SPHL_LightProbe", (SimpleUniform)0);

            Matrix4f worldMatrix;
            worldMatrix.Translate(sphl.position.x, sphl.position.y, sphl.position.z);
            sph_renderer.RenderMesh(sphl.sph_model, worldMatrix);

            if (!Interface.ssphh.enableBasicShowSPHLs) {
                Matrix4f identityMatrix;
                rc.shaderProgram->ApplyUniform("SPHL_LightProbeMode", (SimpleUniform)1);
                rc.shaderProgram->ApplyUniform("WorldMatrix", (SimpleUniform)identityMatrix);
                // Render light probe cube maps
                float angles[3] = { 270.0f, 135.0f, 45.0f };
                for (auto& a : angles)
                    a *= FX_F32_DEGREES_TO_RADIANS;
                float R = 1.0f;
                float S = 0.5f;
                Matrix4f lpWorldMatrix[3];
                rc.shaderProgram->Use();
                for (int i = 0; i < 3; i++) {

                    // glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);
                    //if (i == 0)
                    //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, enviroCubeTexture3.GetTextureId());
                    //else if (i == 1)
                    //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId);
                    //else if (i == 2)

                    glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);

                    //sph_renderer.RenderMesh(sphl.lightProbeModel, lpWorldMatrix[i]);
                    glutCubeMap(
                        sphl.position.x + R * cos(angles[i]),
                        sphl.position.y + R * sin(angles[i]),
                        sphl.position.z,
                        S, vloc, tloc);
                }
            }
        }

        glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glActiveTexture(GL_TEXTURE0);
    }

    if (0) //(!Interface.ssphh.enableBasicShowSPHLs)
    {
        for (auto& sphl : ssg.ssphhLights) {
            double S = 0.25;
            double R = 1.0;
            glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.coronaLightProbeTexture.GetTexture());
            glutCubeMap(
                sphl.position.x + R * 0.707 + S * 1,
                sphl.position.y - R * 0.707,
                sphl.position.z,
                S, vloc, tloc);
            glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.sphLightProbeTexture.GetTexture());
            glutCubeMap(
                sphl.position.x + R * 0.707 + S * 3,
                sphl.position.y - R * 0.707,
                sphl.position.z,
                S, vloc, tloc);
            glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.hierLightProbeTexture.GetTexture());
            glutCubeMap(
                sphl.position.x + R * 0.707 + S * 5,
                sphl.position.y - R * 0.707,
                sphl.position.z,
                S, vloc, tloc);

            //auto i = 0;
            //for (auto & hier : sphl.hierarchies)
            //{
            //	GLuint texture = 0;

            //	texture = hier.debugLightProbe.texture.GetTexture();
            //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            //	glutCubeMap(
            //		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
            //		sphl.position.y - R * 0.707 - S * 2,
            //		sphl.position.z,
            //		S, vloc, tloc);

            //	texture = hier.debugSphLightProbe.texture.GetTexture();
            //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            //	glutCubeMap(
            //		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
            //		sphl.position.y - R * 0.707 - S * 4,
            //		sphl.position.z,
            //		S, vloc, tloc);

            //	texture = ssg.ssphhLights[hier.index].coronaLightProbeTexture.GetTexture();
            //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            //	glutCubeMap(
            //		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
            //		sphl.position.y - R * 0.707 - S * 6,
            //		sphl.position.z,
            //		S, vloc, tloc);

            //	texture = ssg.ssphhLights[hier.index].sphLightProbeTexture.GetTexture();
            //	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            //	glutCubeMap(
            //		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
            //		sphl.position.y - R * 0.707 - S * 8,
            //		sphl.position.z,
            //		S, vloc, tloc);
            //	i++;
            //}

            glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
    }
    sph_renderer.RestoreGLState();
}

void SSPHH_Application::RenderGL11Hierarchies()
{
    if (Interface.ssphh.enableShowHierarchies) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glMultMatrixf(defaultRenderConfig.projectionMatrix.const_ptr());
        glMultMatrixf(defaultRenderConfig.cameraMatrix.const_ptr());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        vector<SimpleVertex> vertices;
        for (int i = 0; i < ssg.ssphhLights.size(); i++) {
            auto& sphl = ssg.ssphhLights[i];
            if (!sphl.enabled)
                continue;

            Color3f color1;
            Color3f color2;

            for (int j = 0; j < ssg.ssphhLights.size(); j++) {
                if (i == j || j < 0)
                    continue;

                float h = 0.0f; // clamp(sphl.hierarchies[j].percentVisible, 0.0f, 1.0f) * 0.5f + 0.5f;
                float s = sphl.enabled ? 1.0f : 0.5f;
                float l;
                if (i >= Interface.ssphh.HierarchiesMaxSphls)
                    l = 0.25f;
                else
                    l = 0.5f;

                SimpleVertex v1, v2;
                v1.color = HLSToRGBf(h, l, s).ToColor4();
                v1.position = ssg.ssphhLights[i].position.xyz();
                v2.color = v1.color;
                v2.position = ssg.ssphhLights[j].position.xyz();

                v1.position += Vector3f(0.0f, -0.5f, 0.0f);
                v2.position += Vector3f(0.0f, 0.5f, 0.0f);

                vertices.push_back(v1);
                vertices.push_back(v2);
            }
            SimpleVertex v1, v2;
            v1.color = sphl.E0 * sphl.GetCoefficientColor(0, 0);
            v2.color = v1.color;
            v1.position = sphl.position.xyz() - Vector3f(0.0f, 0.5f, 0.0f);
            v2.position = sphl.position.xyz() + Vector3f(0.0f, 0.5f, 0.0f);
            vertices.push_back(v1);
            vertices.push_back(v2);
        }

        //glLineWidth(4.0f);
        glEnable(GL_LINE_SMOOTH);
        glBegin(GL_LINES);
        for (auto& v : vertices) {
            glColor4fv(v.color.const_ptr());
            glVertex3fv(v.position.const_ptr());
        }
        glEnd();
        //glLineWidth(1.0f);
        glDisable(GL_LINE_SMOOTH);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    //glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    //glLoadIdentity();
    //glMultMatrixf(rc.projectionMatrix.const_ptr());
    //glMultMatrixf(rc.cameraMatrix.const_ptr());

    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glLoadIdentity();
    //for (auto & it : sphls)
    //{
    //	auto & sphl = it.second;
    //	if (!sphl.enabled) continue;

    //	glPushMatrix();
    //	glTranslatef(sphl.position.X, sphl.position.y, sphl.position.z);
    //	glutSolidSphere(1.0, 16, 16);
    //	glPopMatrix();
    //}
    //glPopMatrix();
    //glMatrixMode(GL_PROJECTION);
    //glPopMatrix();
    //glMatrixMode(GL_MODELVIEW);
}

void SSPHH_Application::RenderTest1SunShadows()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    int w = rectShadowRenderConfig.viewportRect.w;
    int h = rectShadowRenderConfig.viewportRect.h;
    glClear(GL_DEPTH_BUFFER_BIT);
    rectShadowRenderConfig.viewportRect.x = (GLsizei)(screenWidth - 256);
    rectShadowRenderConfig.viewportRect.y = 0;
    rectShadowRenderConfig.viewportRect.w = 256;
    rectShadowRenderConfig.viewportRect.h = 256;
    rectShadowRenderConfig.clearColorBuffer = false;
    rectShadowRenderConfig.renderToFBO = false;
    ssg.AdvancedRender(rectShadowRenderConfig);
    rectShadowRenderConfig.viewportRect.w = w;
    rectShadowRenderConfig.viewportRect.h = h;
    rectShadowRenderConfig.viewportRect.x = 0;
    rectShadowRenderConfig.viewportRect.y = 0;
}

void SSPHH_Application::RenderTest2SphereCubeMap()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    Matrix4f cameraMatrix = Interface.inversePreCameraMatrix * ssg.camera.viewMatrix;
    Vector3f cameraPosition(cameraMatrix.m14, cameraMatrix.m24, cameraMatrix.m34);
    int s = 128;
    SimpleRenderConfiguration& cubeRC = gles30CubeMap.GetRenderConfig();

    cubeRC.clearColorBuffer = false;
    cubeRC.viewportRect.x = 0;
    cubeRC.viewportRect.y = 0;
    cubeRC.preCameraMatrix = Interface.inversePreCameraMatrix;
    cubeRC.postCameraMatrix.LoadIdentity();
    cubeRC.useZOnly = false;
    cubeRC.useMaterials = true;
    cubeRC.viewportRect.w = s;
    cubeRC.viewportRect.h = s;
    cubeRC.postCameraMatrix = ssg.spheres[1].transform;
    cubeRC.useSceneCamera = true;
    cubeRC.isCubeMap = true;
    // gles30CubeMap.Render();
}

void SSPHH_Application::RenderTest3EnviroCubeMap()
{
    glutSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

    SimpleProgramPtr program = renderer2.FindProgram("glut", "UnwrappedCubeMap");
    if (program != nullptr) {
        program->Use();
        GLint tloc = program->GetAttribLocation("aTexCoord");
        GLint vloc = program->GetAttribLocation("aPosition");
        SimpleUniform orthoProjectionMatrix = Matrix4f().Ortho2D(0.0f, screenWidth, 0.0f, screenHeight);
        SimpleUniform identityMatrix = Matrix4f().LoadIdentity();
        program->ApplyUniform("uCubeTexture", SimpleUniform(0));
        program->ApplyUniform("ProjectionMatrix", orthoProjectionMatrix);
        program->ApplyUniform("CameraMatrix", identityMatrix);
        program->ApplyUniform("WorldMatrix", identityMatrix);
        glutBindTextureAndSampler(0, GL_TEXTURE_CUBE_MAP, enviroCubeTexture3.GetTextureId(), ssg.environment.enviroColorMapSamplerId);
        glutUnwrappedCubeMap(0, 0, 256, vloc, tloc);
        glutBindTextureAndSampler(0, GL_TEXTURE_CUBE_MAP, 0, 0);
        glUseProgram(0);
    }
}

void SSPHH_Application::SavePbskyTextures()
{
    // Save PB Sky PPMs
    hflog.info("%s(): saving pbsky ppm texture maps", __FUNCTION__);

    Viperfish::StopWatch stopwatch;
    ssg.environment.pbsky.generatedCylMap.savePPMRaw("pbsky_cylmap.ppm");
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_0.ppm", 0);
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_1.ppm", 1);
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_2.ppm", 2);
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_3.ppm", 3);
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_4.ppm", 4);
    ssg.environment.pbsky.generatedCubeMap.savePPMRaw("pbsky_cubemap_5.ppm", 5);
    stopwatch.Stop();
    hflog.info("%s(): saving pbsky ppm texture maps took %4.2f seconds", __FUNCTION__, stopwatch.GetSecondsElapsed());
}

void SSPHH_Application::RenderSkyBox()
{
    static GLfloat size = 500.0f;
    static GLfloat v[] = {
        -size, size, -size,
        size, size, -size,
        size, -size, -size,
        -size, -size, -size,
        size, size, size,
        -size, size, size,
        -size, -size, size,
        size, -size, size
    };

    static GLfloat texcoords[] = {
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    static GLfloat buffer[] = {
        -size, size, -size, -1.0f, 1.0f, -1.0f,
        size, size, -size, 1.0f, 1.0f, -1.0f,
        size, -size, -size, 1.0f, -1.0f, -1.0f,
        -size, -size, -size, -1.0f, -1.0f, -1.0f,
        size, size, size, 1.0f, 1.0f, 1.0f,
        -size, size, size, -1.0f, 1.0f, 1.0f,
        -size, -size, size, -1.0f, -1.0f, 1.0f,
        size, -size, size, 1.0f, -1.0f, 1.0f
    };

    static GLushort indices[] = {
        // FACE 0
        7, 4, 1,
        1, 2, 7,
        // FACE 1
        3, 0, 5,
        5, 6, 3,
        // FACE 2
        1, 4, 5,
        5, 0, 1,
        // FACE 3
        7, 2, 3,
        3, 6, 7,
        // FACE 5
        6, 5, 4,
        4, 7, 6,
        // FACE 4
        2, 1, 0,
        0, 3, 2
    };

    static GLuint abo = 0;
    static GLuint eabo = 0;
    GLuint vbo = 0;
    GLint vloc = -1;
    GLint tloc = -1;
    GLuint program = 0;
    GLint uCubeTexture = -1;
    GLint uWorldMatrix = -1;
    GLint uCameraMatrix = -1;
    GLint uProjectionMatrix = -1;
    GLint uSunE0 = -1;
    GLint uSunDirTo = -1;
    GLint uGroundE0 = -1;
    GLint uIsHemisphere = -1;
    GLint uToneMapScale = -1;
    GLint uToneMapExposure = -1;
    GLint uToneMapGamma = -1;

    auto p = renderer2.FindProgram("skybox", "skybox");
    if (p) {
        program = p->GetProgram();
        uCubeTexture = p->GetUniformLocation("uCubeTexture");
        uWorldMatrix = p->GetUniformLocation("WorldMatrix");
        uCameraMatrix = p->GetUniformLocation("CameraMatrix");
        uProjectionMatrix = p->GetUniformLocation("ProjectionMatrix");
        uSunE0 = p->GetUniformLocation("SunE0");
        uGroundE0 = p->GetUniformLocation("GroundE0");
        uSunDirTo = p->GetUniformLocation("SunDirTo");
        uIsHemisphere = p->GetUniformLocation("IsHemisphere");
        uToneMapScale = p->GetUniformLocation("ToneMapScale");
        uToneMapExposure = p->GetUniformLocation("ToneMapExposure");
        uToneMapGamma = p->GetUniformLocation("ToneMapGamma");
    }

    if (program == 0)
        return;

    vloc = glGetAttribLocation(program, "aPosition");
    tloc = glGetAttribLocation(program, "aTexCoord");

    if (abo == 0) {
        glGenBuffers(1, &abo);
        glGenBuffers(1, &eabo);
        glBindBuffer(GL_ARRAY_BUFFER, abo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    glUseProgram(program);
    if (uToneMapScale >= 0)
        glUniform1f(uToneMapScale, 2.5f * powf(2.0f, ssg.environment.toneMapScale));
    if (uToneMapExposure >= 0)
        glUniform1f(uToneMapExposure, 2.5f * powf(2.0f, ssg.environment.toneMapExposure));
    if (uToneMapGamma >= 0)
        glUniform1f(uToneMapGamma, ssg.environment.toneMapGamma);
    if (uCubeTexture >= 0) {
        glutBindTextureAndSampler(ssg.environment.pbskyColorMapUnit, GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId, ssg.environment.pbskyColorMapSamplerId);
        glUniform1i(uCubeTexture, ssg.environment.pbskyColorMapUnit);
    }
    if (uProjectionMatrix >= 0) {
        Matrix4f projectionMatrix = ssg.camera.projectionMatrix;
        glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, projectionMatrix.const_ptr());
    }
    if (uCameraMatrix >= 0) {
        Matrix4f viewMatrix = Interface.inversePreCameraMatrix * ssg.camera.viewMatrix;
        viewMatrix.m14 = viewMatrix.m24 = viewMatrix.m34 = viewMatrix.m41 = viewMatrix.m42 = viewMatrix.m43 = 0.0f;
        glUniformMatrix4fv(uCameraMatrix, 1, GL_FALSE, viewMatrix.const_ptr());
    }
    if (uWorldMatrix >= 0) {
        Matrix4f worldMatrix;
        glUniformMatrix4fv(uWorldMatrix, 1, GL_FALSE, worldMatrix.const_ptr());
    }
    if (uSunDirTo >= 0)
        glUniform3fv(uSunDirTo, 1, ssg.environment.curSunDirTo.const_ptr());
    if (uSunE0 >= 0)
        glUniform4fv(uSunE0, 1, ssg.environment.pbsky.GetSunDiskRadiance().const_ptr());
    if (uGroundE0 >= 0)
        glUniform4fv(uGroundE0, 1, ssg.environment.pbsky.GetGroundRadiance().const_ptr());
    if (uIsHemisphere >= 0)
        glUniform1i(uIsHemisphere, ssg.environment.isHemisphere);

    glBindBuffer(GL_ARRAY_BUFFER, abo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eabo);
    glVertexAttribPointer(vloc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void*)0);
    glVertexAttribPointer(tloc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (const void*)12);
    if (vloc >= 0)
        glEnableVertexAttribArray(vloc);
    if (tloc >= 0)
        glEnableVertexAttribArray(tloc);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    if (vloc >= 0)
        glDisableVertexAttribArray(vloc);
    if (tloc >= 0)
        glDisableVertexAttribArray(tloc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void SSPHH_Application::SaveScreenshot()
{
    if (Interface.saveScreenshot) {
        Interface.saveScreenshot = false;
        glFinish();

        Image3ub image((int)screenWidth, (int)screenHeight);

        string filename = GetPathTracerSphlRenderName(
            Interface.sceneName,
            Interface.ssphh.enableKs,
            Interface.ssphh.REF_MaxRayDepth,
            Interface.ssphh.REF_PassLimit,
            Interface.ssphh.MaxDegrees);
        filename += ".ppm";
        Interface.ssphh.lastSphlRenderPath = filename;
        glReadPixels(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight, GL_RGB, GL_UNSIGNED_BYTE, (void*)image.getPixels(0)->const_ptr());
        image.savePPMi(filename, 1.0f, 0, 255, 0, true);
    }
}

string SSPHH_Application::GetPathTracerName(const string& sceneName, bool ks, int mrd, int pl)
{
    ostringstream ostr;
    ostr << sceneName;
    ostr << "_" << setw(2) << setfill('0') << mrd;
    ostr << "_" << setw(2) << setfill('0') << pl;
    if (ks)
        ostr << "_Ks";
    return ostr.str();
}

string SSPHH_Application::GetSphlRenderName(const string& sceneName, int md)
{
    ostringstream ostr;
    ostr << sceneName;
    ostr << "_sphlrender_" << setw(2) << setfill('0') << md;
    return ostr.str();
}

string SSPHH_Application::GetPathTracerSphlRenderName(const string& sceneName, bool ks, int mrd, int pl, int md)
{
    ostringstream ostr;
    ostr << sceneName;
    ostr << "_" << setw(2) << setfill('0') << mrd;
    ostr << "_" << setw(2) << setfill('0') << pl;
    if (ks)
        ostr << "_Ks";
    ostr << "_sphlrender_";
    ostr << setw(2) << setfill('0') << md;
    return ostr.str();
}

string SSPHH_Application::GetStatsName(const string& sceneName, bool ks, int mrd, int pl, int md)
{
    ostringstream ostr;
    ostr << sceneName;
    if (ks)
        ostr << "_Ks";
    ostr << "_" << setw(2) << setfill('0') << mrd;
    ostr << "_" << setw(2) << setfill('0') << pl;
    ostr << "_" << setw(2) << setfill('0') << md;
    return ostr.str();
}

void SSPHH_Application::GI_ScatterJobs()
{
}

int SSPHH_Application::GI_GatherJobs()
{
    int numScattered = ssphhUf.GetNumScatteredJobs();
    int numFinished = ssphhUf.GetNumFinishedJobs();
    ostringstream ostr;
    ostr << "Scattered " << setw(2) << numScattered << "/";
    ostr << "Finished  " << setw(2) << numFinished;
    Interface.ssphh.gi_status = ostr.str();
    if (numFinished > 0) {
        if (numScattered > 0) {
            return 0;
        }
        map<string, CoronaJob> jobs;
        ssphhUf.GetFinishedJobs(jobs);

        int numVIZ = 0;
        int numGEN = 0;
        int numREF = 0;
        for (auto& job : jobs) {
            if (job.second.IsVIZ())
                numVIZ++;
            if (job.second.IsGEN())
                numGEN++;
            if (job.second.IsREF())
                numREF++;
            // All we get are SPHs, so we do not want to cause a memory
            // issue by reading images from disc that don't exist
            GI_ProcessGatherJob(job.second);
        }

        if (numGEN)
            ssg.ssphh.GEN();
        if (numVIZ)
            ssg.ssphh.VIZ();
        ssg.ssphh.HIER();
    }
    return numScattered;
}

bool SSPHH_Application::GI_ProcessJob(CoronaJob& job)
{
    FilePathInfo fpi(job.GetOutputPath());
    if (fpi.DoesNotExist()) {
        hflog.error("%s(): Could not find rendered light probe %s.ppm", __FUNCTION__, job.GetOutputPath().c_str());
        return false;
    }

    int sendLight = -1;
    int recvLight = -1;
    if (job.IsGEN()) {
        sendLight = job.GetGENLightIndex();
    } else if (job.IsVIZ()) {
        sendLight = job.GetVIZSendLightIndex();
        recvLight = job.GetVIZRecvLightIndex();
    }

    SimpleSSPHHLight& sphl = ssg.ssphhLights[sendLight];
    Sph4f sph;
    if (job.IsVIZ()) {
        sphl.vizgenLightProbes[recvLight].loadPPM(fpi.path);
        sphl.vizgenLightProbes[recvLight].convertRectToCubeMap();
        sphl.LightProbeToSph(sphl.vizgenLightProbes[recvLight], sph.msph);
        job.CopySPH(sph);
        return true;
    } else if (job.IsGEN()) {
        sphl.ReadCoronaLightProbe(job.GetOutputPath());
        sphl.SaveCoronaLightProbe(job.GetName() + "_sph.ppm");
        if (ssg.ssphh.saveJSONs)
            sphl.SaveJsonSph(job.GetName() + "_sph.json");

        sphl.vizgenLightProbes[sendLight].loadPPM(fpi.path);
        sphl.vizgenLightProbes[sendLight].convertRectToCubeMap();
        sphl.LightProbeToSph(sphl.vizgenLightProbes[sendLight], sph.msph);
        job.CopySPH(sph);
        sphl.UploadLightProbe(sphl.vizgenLightProbes[sendLight], sphl.hierLightProbeTexture);
        return true;
    }
    return false;
}

bool SSPHH_Application::GI_ProcessGatherJob(CoronaJob& job)
{
    int sendLight = -1;
    int recvLight = -1;
    if (job.IsGEN()) {
        sendLight = job.GetGENLightIndex();
    } else if (job.IsVIZ()) {
        sendLight = job.GetVIZSendLightIndex();
        recvLight = job.GetVIZRecvLightIndex();
    }

    SimpleSSPHHLight& sphl = ssg.ssphhLights[sendLight];
    Sph4f sph;
    job.CopySPHToSph4f(sph);
    if (job.IsVIZ()) {
        sphl.SphToLightProbe(sph.msph, sphl.vizgenLightProbes[recvLight]);
        return true;
    } else if (job.IsGEN()) {
        sphl.SphToLightProbe(sph.msph, sphl.vizgenLightProbes[sendLight]);
        sphl.UploadLightProbe(sphl.vizgenLightProbes[sendLight], sphl.hierLightProbeTexture);
        return true;
    }
    return false;
}

void SSPHH_Application::RunJob(CoronaJob& job)
{
    job.Start(coronaScene, ssg);
    GI_ProcessJob(job);
}