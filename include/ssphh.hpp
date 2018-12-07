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
#ifndef SSPHH_HPP
#define SSPHH_HPP

#include <fluxions.hpp>
#include <fluxions_corona_scene.hpp>
#include <fluxions_renderer.hpp>
#include <hatchetfish.hpp>
#include <kasl_python.hpp>
#include <viperfish.hpp>
//#include <fluxions_pbsky.hpp>
//#include <kasl_python.hpp>
#include <fluxions_sphl.hpp>
#include <imgui.h>
#include <ssphh_ppmcompare.hpp>
#include <ssphh_unicornfish.hpp>

using namespace Viperfish;
using namespace Fluxions;

class SSPHH_Application : public Widget {
private:
    // Physically Based Code
    //Color4f groundRadiance;
    //Color4f sunDiskRadiance;
    double pbskyAge = 0.0;
    double pbskyMaxAge = 5.0;
    //double pbskyLatitude = 38.9;
    //double pbskyLongitude = -104.8;
    //bool recomputeSkyCube;
    bool isSkyComputing;
    bool isSkyComputed;
    time_t pbsky_localtime;
    double pbsky_timeOffsetInSeconds = 0.0;
    //void pbskyComputeCubemap();
    SimpleTexture PBSkyCubeMap;
    SimpleRenderer_GLushort Skybox;
    void RenderSkyBox();
    ////////////////////////

    GLfloat aspect = 1.0f;
    GLfloat fovy = 45.0f;
    // GLfloat nearVal = 0.001f;
    // GLfloat farVal = 1000.0f;
    GLfloat screenX = 0.0f;
    GLfloat screenY = 0.0f;
    GLfloat screenWidth = 0.0f;
    GLfloat screenHeight = 0.0f;

    float rotX = 0.0;
    float rotY = 0.0;
    float rotZ = 0.0;

    //string sceneFilename = "resources/scenes/test_texture_scene/test_outdoor_scene.scn";
    //string sceneFilename = "resources/scenes/test_texture_scene/test_mitsuba_scene.scn";
    //string sceneFilename = "resources/scenes/test_texture_scene/test_indoor_scene.scn";
    string sceneFilename = "resources/scenes/test_texture_scene/test_gallery_scene.scn";
    //string sceneFilename = "resources/scenes/test_texture_scene/simple_inside_scene.scn";
    //string sceneFilename = "resources/scenes/rungholt_scene/rungholt.scn";

    SimpleRenderConfiguration defaultRenderConfig;
    SimpleRenderConfiguration gbufferRenderConfig;
    SimpleRenderConfiguration cubeShadowRenderConfig;
    SimpleRenderConfiguration cubeEnvMapRenderConfig;
    SimpleRenderConfiguration rectShadowRenderConfig;
    SimpleRenderConfiguration rectEnvMapRenderConfig;
    SimpleRenderConfiguration uberShaderRenderConfig;

    SamplerObject defaultCubeTextureSampler;
    SamplerObject default2DTextureSampler;
    SamplerObject defaultShadowCubeTextureSampler;
    SamplerObject defaultShadow2DTextureSampler;

    //SimpleTexture enviroCubeTexture1;
    //SimpleTexture enviroCubeTexture2;
    SimpleTexture enviroCubeTexture3;

    //TMatrix4<GLfloat> screenPerspMatrix;
    TMatrix4<GLfloat> screenOrthoMatrix;

    Fluxions::SimpleRenderer_GLuint renderer;
    Fluxions::CoronaDatabase sceneDB;
    Fluxions::SimpleSceneGraph ssg;
    Fluxions::CoronaSceneFile coronaScene;
    vector<Fluxions::CoronaJob> coronaJobs;
    Fluxions::SimpleGLES30Renderer gles30;

    Fluxions::Renderer renderer2;

    Fluxions::SimpleGLES30Renderer gles30CubeMap;

    // SPHL code
    SimpleGLES30Renderer sph_renderer;
    SimpleGeometryMesh ssphh_hierarchy_mesh;
    FxModel geosphere;
    // Sphl sphl;
    map<int, Sphl> sphls;
    bool coefs_init = false;
    // End SPHL code

    struct InterfaceInfo {
        Matrix4f projectionMatrix;
        Matrix4f preCameraMatrix;
        Matrix4f inversePreCameraMatrix;
        Matrix4f postCameraMatrix;
        Matrix4f inversePostCameraMatrix;
        Matrix4f finalCameraMatrix;
        Matrix4f inverseFinalCameraMatrix;
        Vector4f cameraPosition;
        Vector4f cameraOrbit;

        double moveX = 0.0;
        double moveY = 0.0;
        double moveZ = 0.0;
        double turnX = 0.0;
        double turnY = 0.0;
        double turnZ = 0.0;
        double moveRate = 0.5;
        double turnRate = 1.0;

        bool enableOrbit = false;
        bool enableSunCycle = false;
        bool usePBSKY = true;
        bool drawSkyBox = true;
        bool saveScreenshot = false;
        bool captureShadows = false;

        bool moveLeft = false;
        bool moveRight = false;
        bool moveForward = false;
        bool moveBackward = false;
        bool moveDown = false;
        bool moveUp = false;
        bool yawLeft = false;
        bool yawRight = false;
        bool pitchDown = false;
        bool pitchUp = false;
        bool rollLeft = false;
        bool rollRight = false;

        bool increaseLatitude = false;
        bool increaseLongitude = false;
        bool decreaseLatitude = false;
        bool decreaseLongitude = false;

        bool showHelp = false;
        bool showHUD = false;
        bool showMainHUD = false;
        bool showDeferredHUD = false;
        bool showImGui = true;

        struct TOOLSINFO {
            bool showMaterialEditor = false;
            bool showSphlEditor = false;
            bool showScenegraphEditor = false;
            bool showDebugView = false;
            bool showSSPHHWindow = true;
            bool showUnicornfishWindow = true;
            bool showRenderConfigWindow = false;
            int shaderDebugChoice = 17;
            int shaderDebugLight = -1;
            int shaderDebugSphl = -1;
            int shaderDebugChoices[4] = { 0, 0, 0, 0 };
            vector<string> gl_extensions;
            vector<const char*> gl_extensions_cstr;
            int gl_extensions_curitem = 0;
            string gl_version;
            string gl_renderer;
            bool showGLInfo = false;
        } tools;

        struct SSGINFO {
            bool showEnvironment = false;
            bool showEnvironmentDetails = false;
            bool showGeometry = false;
            bool showGeometryDetails = false;
            bool showSpheres = false;
            bool showSphereDetails = false;
            bool showPointLights = false;
            bool showPointLightDetails = false;

            float cameraFOV = 45.0f;

            vector<pair<int, bool>> geometryCollapsed;
        } ssg;

        struct MATERIALSINFO {
            struct MtlValuePtrs {
                float* PBm = nullptr;
                float* PBk = nullptr;
                float* PBior = nullptr;
                float* PBKdm = nullptr;
                float* PBKsm = nullptr;
                float* PBGGXgamma = nullptr;
                float* Kd = nullptr;
                float* Ks = nullptr;
                float* Ke = nullptr;
                float* Ka = nullptr;
                const char* map_Kd = nullptr;
                const char* map_Ks = nullptr;
                const char* map_normal = nullptr;
                const char* map_bump = nullptr;
            };
            bool showMaps = false;
            bool showMtls = true;
            map<string, bool> mtllibsCollapsed;
            map<string, bool> mtlsCollapsed;
            map<string, SimpleMaterial*> mtls;
            map<string, MtlValuePtrs> mtls_ptrs;
            //vector<pair<bool, vector<int, bool>>> mtlsCollapsed;
        } mtls;

        struct SSPHHWINDOW {
            bool enabled = false;
            static const int MaxSPHLs = 16;

            enum class SSPHH_Status {
                SendUfMessage = 0,
                RecvUfMessage = 1,
                UploadToGpu = 2,
                Finished
            };

            bool enableHQ = false;
            bool enableHDR = false;
            bool enableKs = true;
            bool enableREF = true;
            bool enableREFCubeMap = false;
            int numSphls = 0;
            map<int, SSPHH_Status> ssphhStatus;
            bool enableShadowColorMap = false;
            bool enableShowHierarchies = false;
            bool enableShowSPHLs = true;
            bool enableBasicShowSPHLs = true;
            vector<double> gen_times;
            vector<double> viz_times;
            bool HierarchiesIncludeSelf = true;
            bool HierarchiesIncludeNeighbors = true;
            int HierarchiesMaxSphls = MaxSphlLights;
            bool HierarchiesGeneratePPMs = false;
            int MaxDegrees = MaxSphlDegree;
            int REF_MaxRayDepth = 5;
            int REF_PassLimit = 1;
            int VIZ_MaxRayDepth = 3;
            int VIZ_PassLimit = 1;
            int GEN_MaxRayDepth = 3;
            int GEN_PassLimit = 1;
            int LightProbeSizeChoice = 4;
            int LightProbeSize = 2 << 4;
            int ShadowSizeChoice = 6;
            int ShadowSize = 2 << 6;
            bool GEN_IgnoreCache = false;
            bool VIZ_IgnoreCache = false;
            bool REF_IgnoreCache = false;
            bool genProductsIgnoreCache = false;
            bool ppmcompareIgnoreCache = false;
            bool ppmcompareGenPPMs = false;
            string gi_status;

            string lastREFPath;
            string lastREFCubeMapPath;
            string lastSphlRenderPath;

            double lastREFTime = 0.0;
            double lastREFCubeMapTime = 0.0;
            double lastINITTime = 0.0;
            double lastVIZTime = 0.0;
            double lastGENTime = 0.0;
            double lastHIERTime = 0.0;
            double lastPathTracerTotalEnergy = 0.0;
            double lastSphlRenderTotalEnergy = 0.0;
            double lastDiff1TotalEnergy = 0.0;
            double lastDiff2TotalEnergy = 0.0;
        } ssphh;

        struct UNICORNFISHWINDOW {
            // Uf read from variables / UI write to variables
            vector<string> send_queue;
            vector<const char*> send_queue_items;
            int send_queue_item = 0;

            // Uf write to variables / UI read from variables
            vector<string> recv_queue;
            vector<const char*> recv_queue_items;
            int recv_queue_item = 0;

            bool uf_isinit = false; // one time flag to tell service if it has started or not
            bool uf_stop = false; // one time flag to tell service to stop
            UfType uf_type;
            string endpoint;
            string service;

            bool standalone_client = true;
            bool standalone_broker = true;
            bool standalone_worker = true;

            thread broker_thread;
            thread worker_thread;
            thread client_thread;

            bool windowInit = false;
        } uf;

        struct RENDERCONFIGWINDOW {
            int sunShadowMapSizeChoice = 10;
            int sunShadowMapSize = 2 << 10;
            float sunShadowMapZoom = 1.0;
        } renderconfig;

        bool recomputeSky = true;

        bool saveCoronaSCN = false;
        bool saveCoronaCubeMapSCN = false;
        bool renderCoronaSCN = false;
        bool renderCoronaCubeMapSCN = false;

        bool sphl_checkboxes[32][4] = { false };

        double lastRenderConfigLoadTime = 0.0;
        double lastScenegraphLoadTime = 0.0;

        string sceneName;
    };

    struct HUDInfo {
        double pbskyTime = 0.0;
        double gbufferPassTime = 0.0;
        double deferredPassTime = 0.0;
        double totalRenderTime = 0.0;
        double onUpdateTime = 0.0;

        string glRendererString = "";
        string glVersionString = "";
        string glVendorString = "";
        vector<string> glLastDebugMessages;
    };

    void SavePbskyTextures();

    void AdvanceSunClock(double numSeconds, bool recomputeSky = false);

    void imguiSphlAdd();
    void imguiSphlDelete();
    void imguiSphlRandomize();
    void imguiSphlIncreaseDegrees();
    void imguiSphlDecreaseDegrees();
    void imguiSphlCoronaToSPH(size_t which);
    void imguiSphlSPHtoCubeMap(size_t which);
    void imguiSphlSaveToOBJ();

    void imguiToolsTakeScreenshot();
    void imguiToolsSaveStats();
    void imguiToolsResetStats();

    void imguiShowMenuBar();
    void imguiShowToolWindow();
    void imguiShowRenderConfigWindow();
    void imguiShowScenegraphEditor();
    void imguiShowSphlEditor();
    void imguiShowMaterialEditor();
    void imguiCoronaControls();
    void imguiCoronaGenerateSCN();
    void imguiCoronaGenerateSphlVIZ();
    void imguiCoronaGenerateSphlINIT();
    void imguiCoronaGenerateSphlHIER();
    void imguiCoronaGenerateSphlGEN();
    void imguiCoronaGenerateSky();
    void imguiCoronaCheckCache();
    void imguiCoronaGenerateREF();
    void imguiCoronaDeleteCache();
    void imguiCoronaGenerateTestProducts();
    void imguiCoronaGenerateCompareProduct(bool ks, int mrd, int pl, int md);
    void imguiCoronaDeleteTestProducts();

    void imgui2NSizeSlider(const char* desc, int* choice, int* size, int minvalue, int maxvalue);

    void imguiUfShowWindow();
    void imguiShowSSPHHWindow();

    void imguiUfBrokerControls();
    void imguiUfWorkerEchoControls();
    void imguiUfWorkerCoronaControls();
    void imguiUfClientGLES20Controls();
    void imguiUfClientGLES30Controls();

    void imguiMatrix4fEditControl(int id, Matrix4f& m);

    void RenderHUD();
    void RenderMainHUD();
    void RenderImGuiHUD();
    void RenderHelp();
    void RenderDeferredHUD();

    void SaveScreenshot();

    static string GetPathTracerName(const string& sceneName, bool ks, int mrd, int pl);
    static string GetSphlRenderName(const string& sceneName, int md);
    static string GetPathTracerSphlRenderName(const string& sceneName, bool ks, int mrd, int pl, int md);
    static string GetStatsName(const string& sceneName, bool ks, int mrd, int pl, int md);

    void SetupRenderGLES30();
    void UpdateSPHLs();
    void DirtySPHLs();

    const int MAX_RENDER_MODES = 3;
    int renderMode = 2;
    void RenderFixedFunctionGL();
    void RenderGLES20();
    void RenderGLES30();
    void RenderGLES30Scene();
    void RenderGLES30Shadows();
    void RenderGLES30SPHLs();
    void RenderGL11Hierarchies();

    void RenderTest1SunShadows();
    void RenderTest2SphereCubeMap();
    void RenderTest3EnviroCubeMap();

    void ParseCommandArguments(const vector<string>& args);
    void InitUnicornfish();
    void KillUnicornfish();

    void GI_ScatterJobs();
    int GI_GatherJobs();
    bool GI_ProcessJob(CoronaJob& job);
    bool GI_ProcessGatherJob(CoronaJob& job);

public:
    HUDInfo my_hud_info;
    //KASL::PythonInterpreter python;

    vector<string> args; // to pull in the argc, argv from main()

    SSPHH_Application();
    ~SSPHH_Application();

    void OnInit(const vector<string>& args) override;
    void OnKill() override;

    virtual void OnUpdate(double timeStamp) override;
    virtual void DoInterfaceUpdate(double deltaTime);
    const Matrix4f& GetCameraMatrix() const;

    const string& GetSceneName() const { return Interface.sceneName; }

    InterfaceInfo Interface;
    int counter = 0;

    //virtual void OnKeyDown(int key);
    //virtual void OnKeyUp(int key);
    //virtual void OnSpecialKeyDown(int key);
    //virtual void OnSpecialKeyUp(int key);

    virtual void OnKeyDown(const string& key, int modifiers) override;
    virtual void OnKeyUp(const string& key, int modifiers) override;

    //virtual void OnMouseMove(int X, int y, int dx, int dy);
    //virtual void OnMouseButtonDown(int X, int y, int button);
    //virtual void OnMouseButtonUp(int X, int y, int button);

    virtual void OnMouseMove(int x, int y) override;
    virtual void OnMouseButtonDown(int button) override;
    virtual void OnMouseButtonUp(int button) override;
    virtual void OnMouseClick(int button, const MouseClickState& mcs) override;
    virtual void OnMouseDoubleClick(int button, const MouseDoubleClickState& mdcs) override;
    virtual void OnMouseDrag(int button, const MouseDragState& mds) override;

    void OnRender() override;
    void OnRenderOverlay() override;

    void ResetScene();
    void UseCurrentTime();
    void InitRenderConfigs();
    void LoadRenderConfigs();
    void LoadScene();

    virtual void OnReshape(int width, int height) override;

    void RunJob(CoronaJob& job);

private:
    void OptimizeClippingPlanes();
    void ReloadRenderConfigs();
    void ReloadScenegraph();
    void StartPython();
};

// extern SSPHH_Application ssphh;
extern shared_ptr<SSPHH_Application> ssphhPtr;
void InitSSPHH();
void KillSSPHH();

#endif