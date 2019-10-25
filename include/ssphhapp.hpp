// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
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
#include <fluxions_renderer.hpp>
#include <hatchetfish.hpp>
#include <damselfish_python.hpp>
#include <viperfish_mouse.hpp>
#include <viperfish_widget.hpp>
//#include <fluxions_pbsky.hpp>
#include <fluxions_sphl.hpp>
#include <imgui.h>
#include <ssphh_ppmcompare.hpp>
#include <ssphh_unicornfish.hpp>
#include <fluxions_ssg_ssphh_renderer_plugin.hpp>
#include <fluxions_simple_ssphh.hpp>
#include <PbskyViewController.hpp>

//#define SSPHH_RENDER_CLASSIC_OPENGL 1
#ifdef SSPHH_RENDER_CLASSIC_OPENGL
#include <GLUT_template.hpp>
#endif

using namespace Vf;
using namespace Fluxions;

namespace SSPHH
{
	class SSPHH_Application : public Widget
	{
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

	public:
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

		//std::string sceneFilename = "resources/scenes/test_texture_scene/test_outdoor_scene.scn";
		//std::string sceneFilename = "resources/scenes/test_texture_scene/test_mitsuba_scene.scn";
		//std::string sceneFilename = "resources/scenes/test_texture_scene/test_indoor_scene.scn";
		std::string sceneFilename = "resources/scenes/test_texture_scene/test_gallery_scene.scn";
		//std::string sceneFilename = "resources/scenes/test_texture_scene/simple_inside_scene.scn";
		//std::string sceneFilename = "resources/scenes/rungholt_scene/rungholt.scn";

		SimpleRenderConfiguration defaultRenderConfig;
		SimpleRenderConfiguration gbufferRenderConfig;
		SimpleRenderConfiguration cubeShadowRenderConfig;
		SimpleRenderConfiguration cubeEnvMapRenderConfig;
		SimpleRenderConfiguration rectShadowRenderConfig;
		SimpleRenderConfiguration rectEnvMapRenderConfig;
		SimpleRenderConfiguration uberShaderRenderConfig;
		SimpleRenderConfiguration foursplitULRenderConfig;
		SimpleRenderConfiguration foursplitURRenderConfig;
		SimpleRenderConfiguration foursplitLLRenderConfig;
		SimpleRenderConfiguration foursplitLRRenderConfig;

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
		//Uf::CoronaDatabase sceneDB;
		Fluxions::SimpleSceneGraph ssg;
		Fluxions::SimpleSSPHH ssphh;
		Fluxions::SSG_SSPHHRendererPlugin* ssgUserData = nullptr;
		Uf::CoronaSceneFile coronaScene;
		std::vector<Uf::CoronaJob> coronaJobs;
		Fluxions::SimpleGLES30Renderer gles30;

		Fluxions::Renderer renderer2;

		Fluxions::SimpleGLES30Renderer gles30CubeMap;

		// SPHL code
		SimpleGLES30Renderer sph_renderer;
		SimpleGeometryMesh ssphh_hierarchy_mesh;
		FxModel geosphere;
		// Sphl sphl;
		std::map<size_t, Sphl> sphls;
		bool coefs_init = false;
		// End SPHL code

		struct InterfaceInfo
		{
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
			bool showImGui = false;

			struct TOOLSINFO
			{
				bool showMaterialEditor = false;
				bool showSphlEditor = false;
				bool showScenegraphEditor = false;
				bool showDebugView = false;
				bool showSSPHHWindow = true;
				bool showTestsWindow = true;
				bool showUnicornfishWindow = true;
				bool showRenderConfigWindow = false;
				int shaderDebugChoice = 17;
				int shaderDebugLight = -1;
				int shaderDebugSphl = -1;
				int shaderDebugChoices[4] = { 0, 0, 0, 0 };
				std::vector<std::string> gl_extensions;
				std::vector<const char*> gl_extensions_cstr;
				int gl_extensions_curitem = 0;
				std::string gl_version;
				std::string gl_renderer;
				bool showGLInfo = false;
			} tools;

			struct SSGINFO
			{
				bool showEnvironment = false;
				bool showEnvironmentDetails = false;
				bool showGeometry = false;
				bool showGeometryDetails = false;
				bool showSpheres = false;
				bool showSphereDetails = false;
				bool showPointLights = false;
				bool showPointLightDetails = false;

				float cameraFOV = 45.0f;

				std::vector<std::pair<int, bool>> geometryCollapsed;

				std::string scenename;

				bool resetScene = false;
				bool saveScene = false;
				bool loadScene = false;
				bool createScene = false;
			} ssg;

			struct MATERIALSINFO
			{
				struct MtlValuePtrs
				{
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
				std::map<std::string, bool> mtllibsCollapsed;
				std::map<std::string, bool> mtlsCollapsed;
				std::map<std::string, SimpleMaterial*> mtls;
				std::map<std::string, MtlValuePtrs> mtls_ptrs;
				//vector<pair<bool, vector<int, bool>>> mtlsCollapsed;
			} mtls;

			struct SSPHHWINDOW
			{
				bool enabled = false;
				static const int MaxSPHLs = 16;

				enum class SSPHH_Status
				{
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
				std::map<int, SSPHH_Status> ssphhStatus;
				bool enableShadowColorMap = false;
				bool enableShowHierarchies = false;
				bool enableShowSPHLs = true;
				bool enableBasicShowSPHLs = true;
				std::vector<double> gen_times;
				std::vector<double> viz_times;
				bool HierarchiesIncludeSelf = true;
				bool HierarchiesIncludeNeighbors = true;
				float HierarchiesMix;
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
				std::string gi_status;

				std::string lastREFPath;
				std::string lastREFCubeMapPath;
				std::string lastSphlRenderPath;

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

			struct UNICORNFISHWINDOW
			{
				// Uf read from variables / UI write to variables
				std::vector<std::string> send_queue;
				std::vector<const char*> send_queue_items;
				int send_queue_item = 0;

				// Uf write to variables / UI read from variables
				std::vector<std::string> recv_queue;
				std::vector<const char*> recv_queue_items;
				int recv_queue_item = 0;

				bool uf_isinit = false; // one time flag to tell service if it has started or not
				bool uf_stop = false;   // one time flag to tell service to stop
				UfType uf_type{ UfType::None };
				std::string endpoint{ "" };
				std::string service{ "" };

				bool standalone_client = true;
				bool standalone_broker = true;
				bool standalone_worker = true;

				std::thread broker_thread;
				std::thread worker_thread;
				std::thread client_thread;

				bool windowInit = false;
			} uf;

			struct RENDERCONFIGWINDOW
			{
				int sunShadowMapSizeChoice = 10;
				int sunShadowMapSize = 2 << 10;
				float sunShadowMapZoom = 1.0;
			} renderconfig;

			struct TESTSWINDOW
			{
				bool bTestSPHLs = false;
				bool bShowSPHLResults = false;
				int saveSphlJSON = 0;
				int readSphlJSON = 0;
				int saveSphlOBJ = 0;
				int saveSphlEXR = 0;
				int readSphlEXR = 0;
				int saveSphlPPM = 0;
				int readSphlPPM = 0;
			} tests;

			bool recomputeSky = true;

			bool saveCoronaSCN = false;
			bool saveCoronaCubeMapSCN = false;
			bool renderCoronaSCN = false;
			bool renderCoronaCubeMapSCN = false;

			bool sphl_checkboxes[32][4] = { {false} };

			double lastRenderConfigLoadTime = 0.0;
			double lastScenegraphLoadTime = 0.0;

			std::string sceneName;
		}; // InterfaceInfo
		float imguiWinX = 64.0f;
		float imguiWinW = 384.0f;

		struct HUDInfo
		{
			double pbskyTime = 0.0;
			double gbufferPassTime = 0.0;
			double deferredPassTime = 0.0;
			double totalRenderTime = 0.0;
			double onUpdateTime = 0.0;

			std::string glRendererString = "";
			std::string glVersionString = "";
			std::string glVendorString = "";
			std::vector<std::string> glLastDebugMessages;
		};

		void AdvanceSunClock(double numSeconds, bool recomputeSky = false);

		void InitImGui();

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
		void imguiCoronaCheckCache();
		void imguiCoronaGenerateREF();
		void imguiCoronaDeleteCache();
		void imguiCoronaGenerateTestProducts();
		void imguiCoronaGenerateCompareProduct(bool ks, int mrd, int pl, int md);
		void imguiCoronaDeleteTestProducts();

		void imgui2NSizeSlider(const char* desc, int* choice, int* size, int minvalue, int maxvalue);

		void imguiShowUfWindow();
		void imguiShowSSPHHWindow();
		void imguiShowTestWindow();

		void imguiUfBrokerControls();
		void imguiUfWorkerEchoControls();
		void imguiUfWorkerCoronaControls();
		void imguiUfClientGLES20Controls();
		void imguiUfClientGLES30Controls();

		void imguiMatrix4fEditControl(int id, Matrix4f& m);

		// PBSKY ModelViewController

	private:
		PbskyViewController* vcPbsky = nullptr;
	public:
		void RegenHosekWilkieSky();
		void SaveHosekWilkieSky();
		void RegenCoronaSky();
		void LoadCoronaSky(bool loadEXR, const std::string& path);


		// TESTS

		void Test();
		void TestSPHLs();

		// RENDERING

		void RenderHUD();
		void RenderMainHUD();
		void RenderImGuiHUD();
		void RenderHelp();
		void RenderDeferredHUD();

		// TASKS

		void SaveScreenshot();
		void ProcessScenegraphTasks();

		static std::string GetPathTracerName(const std::string& sceneName, bool ks, int mrd, int pl);
		static std::string GetSphlRenderName(const std::string& sceneName, int md);
		static std::string GetPathTracerSphlRenderName(const std::string& sceneName, bool ks, int mrd, int pl, int md);
		static std::string GetStatsName(const std::string& sceneName, bool ks, int mrd, int pl, int md);

		void SetupRenderGLES30();

		void DirtySPHLs();
		void UpdateSPHLs();
		void UploadSPHLs();

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

		void ParseCommandArguments(const std::vector<std::string>& args);
		void InitUnicornfish();
		void KillUnicornfish();

		void GI_ScatterJobs();
		int GI_GatherJobs();
		bool GI_ProcessJob(Uf::CoronaJob& job);
		bool GI_ProcessGatherJob(Uf::CoronaJob& job);

	public:
		HUDInfo my_hud_info;
		//Df::PythonInterpreter python;

		std::vector<std::string> cmd_args; // to pull in the argc, argv from main()

		using SharedPtr = std::shared_ptr<SSPHH_Application>;
		using UniquePtr = std::unique_ptr<SSPHH_Application>;

		template <class... _Types>
		static SharedPtr MakeShared(_Types&&... _Args) { return SharedPtr(new SSPHH_Application(std::forward<_Types>(_Args)...)); }
		template <class... _Types>
		static UniquePtr MakeUnique(_Types&&... _Args) { return UniquePtr(new SSPHH_Application(std::forward<_Types>(_Args)...)); }

		SSPHH_Application();
		SSPHH_Application(const std::string& name);
		~SSPHH_Application();

		void OnInit(const std::vector<std::string>& args) override;
		void OnKill() override;

		virtual void OnUpdate(double timeStamp) override;
		virtual void DoInterfaceUpdate(double deltaTime);
		const Matrix4f& GetCameraMatrix() const;

		const std::string& GetSceneName() const { return Interface.sceneName; }

		InterfaceInfo Interface;
		int counter = 0;

		//virtual void OnKeyDown(int key);
		//virtual void OnKeyUp(int key);
		//virtual void OnSpecialKeyDown(int key);
		//virtual void OnSpecialKeyUp(int key);

		virtual void OnKeyDown(const std::string& key, int modifiers) override;
		virtual void OnKeyUp(const std::string& key, int modifiers) override;

		//virtual void OnMouseMove(int X, int y, int dx, int dy);
		//virtual void OnMouseButtonDown(int X, int y, int button);
		//virtual void OnMouseButtonUp(int X, int y, int button);

		virtual void OnMouseMove(int x, int y) override;
		virtual void OnMouseButtonDown(int button) override;
		virtual void OnMouseButtonUp(int button) override;
		virtual void OnMouseClick(int button, const MouseClickState& mcs) override;
		virtual void OnMouseDoubleClick(int button, const MouseDoubleClickState& mdcs) override;
		virtual void OnMouseDrag(int button, const MouseDragState& mds) override;

		void OnPreRender() override;
		void OnRender3D() override;
		void OnRender2D() override;
		void OnRenderDearImGui() override;
		void OnPostRender() override;

		void ResetScene();
		void UseCurrentTime();
		void InitRenderConfigs();
		void LoadRenderConfigs();
		void LoadScene();

		virtual void OnReshape(int width, int height) override;

		void RunJob(Uf::CoronaJob& job);

	private:
		void OptimizeClippingPlanes();
		void ReloadRenderConfigs();
		void ReloadScenegraph();
		void StartPython();
	};
} // namespace SSPHH

// extern SSPHH_Application ssphh;
extern std::shared_ptr<SSPHH::SSPHH_Application> ssphhPtr;
void InitSSPHH();
void KillSSPHH();

#endif
