#include <ssphhapp.hpp>

extern void PrintString9x15(float x, float y, int justification, const char* format, ...);

namespace SSPHH
{
	void SSPHH_Application::OnPreRender() {
		HFLOGDEBUGFIRSTRUN();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void SSPHH_Application::OnRender3D() {
		HFLOGDEBUGFIRSTRUN();

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

		double renderT0 = Hf::Log.getSecondsElapsed();
		if (renderMode == 0) {
			RenderFixedFunctionGL();
		}
		else if (renderMode == 1) {
			RenderGLES20();
		}
		else if (renderMode == 2) {
			RenderGLES30();
		}
		my_hud_info.totalRenderTime = Hf::Log.getSecondsElapsed() - renderT0;

		double renderT1 = Hf::Log.getMillisecondsElapsed();
		double dt = renderT1 - renderT0;
		if (dt < 1000.0)
			Hf::Log.takeStat("frametime", dt);
		SaveScreenshot();
	}

	void SSPHH_Application::OnRender2D() {
		HFLOGDEBUGFIRSTRUN();

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
			// Vector2i split = renderer2.GetDeferredSplitPoint();
			// Recti rect = renderer2.GetDeferredRect();

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
	}

	void SSPHH_Application::OnRenderDearImGui() {
		HFLOGDEBUGFIRSTRUN();

		if (Interface.showImGui) {
			RenderImGuiHUD();
		}
	}

	void SSPHH_Application::OnPostRender() {
		HFLOGDEBUGFIRSTRUN();
	}

	void SSPHH_Application::RenderHUD() {
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
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Renderer: %s", my_hud_info.glRendererString.c_str());
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Vendor:   %s", my_hud_info.glVendorString.c_str());
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Version:  %s", my_hud_info.glVersionString.c_str());

		ypos += 15.0f;
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "OnUpdate() time:    % 4.2f", my_hud_info.onUpdateTime);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Total Render time:  % 4.2f", my_hud_info.totalRenderTime);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "G-Buffer pass time: % 4.2f", my_hud_info.gbufferPassTime);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Deferred pass time: % 4.2f", my_hud_info.deferredPassTime);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Pbsky render time:  % 4.2f", my_hud_info.pbskyTime);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Pbsky RGB min/max:  MIN: % 4.2e -- MAX: % 4.2e",
										 ssg.environment.pbsky.getMinRgbValue(), ssg.environment.pbsky.getMaxRgbValue());
		ypos += 15.0f;
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Render mode: %d %s", renderMode, renderModes[renderMode]);
		ypos += 15.0f;
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m11, cameraMatrix.m12, cameraMatrix.m13, cameraMatrix.m14);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m21, cameraMatrix.m22, cameraMatrix.m23, cameraMatrix.m24);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m31, cameraMatrix.m32, cameraMatrix.m33, cameraMatrix.m34);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "% -2.3f, % -2.3f, % -2.3f, % -2.3f", cameraMatrix.m41, cameraMatrix.m42, cameraMatrix.m43, cameraMatrix.m44);

		Vector4f eye = cameraMatrix * Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "CAMERA: X: % -3.2f, Y: % -3.2f, Z: % -3.2f", eye.x, eye.y, eye.z);
		Vector3f sun = ssg.environment.pbsky.GetSunVector();
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "SUN ANGLES:   AZ: % -3.1f, ALT: % -3.1f",
										 ssg.environment.pbsky.GetSunAzimuth(),
										 ssg.environment.pbsky.GetSunAltitude());
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "SUN POSITION: X: % -2.2f  Y: % -2.2f  Z: % -2.2f", sun.x, sun.y, sun.z);
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "MAP POSITION: LAT: % -3.2f   LONG: % -3.2f", ssg.environment.pbsky.GetLatitude(), ssg.environment.pbsky.GetLongitude());

		Color4f sunDiskRadiance = ssg.environment.pbsky.GetSunDiskRadiance();
		Color4f groundRadiance = ssg.environment.pbsky.GetGroundRadiance();
		// Sun Disk Radiance
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "SUN DISK RADIANCE: R: % -3.1f  G: % -3.1f  B: % -3.2f  AVG: % -3.2f",
										 sunDiskRadiance.r, sunDiskRadiance.g, sunDiskRadiance.b,
										 ssg.environment.pbsky.GetAverageRadiance());

		// Ground Radiance
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "GROUND RADIANCE:   R: % -3.1f  G: % -3.1f  B: % -3.2f",
										 groundRadiance.r, groundRadiance.g, groundRadiance.b);

		float seconds = (float)(ssg.environment.pbsky.getSec() + ssg.environment.pbsky.getSecFract());
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 0, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "DATE: %02d/%02d/%02d %02d:%02d:%02d%.3f  LST: %2.3f",
										 ssg.environment.pbsky.getMonth(),
										 ssg.environment.pbsky.getDay(),
										 ssg.environment.pbsky.getYear(),
										 ssg.environment.pbsky.getHour(),
										 ssg.environment.pbsky.getMin(),
										 ssg.environment.pbsky.getSec(),
										 ssg.environment.pbsky.getSecFract(),
										 ssg.environment.pbsky.getLST());
	}


	void SSPHH_Application::RenderMainHUD() {
#ifdef SSPHH_RENDER_CLASSIC_OPENGL
		glColor3f(1, 1, 1);

		float y = 20.0f;
		PrintString9x15(0, y, 1, "SSPHH 2017");
		y += 15.0f;

		PrintString9x15(0, y, 1, "%.3f fps", gt_Fps);
		y += 15.0f;

		PrintString9x15(0, y, 1, "counter: %d", counter);
		y += 15.0f;

		if (counter == 0)
			PrintString9x15(0, y, FX_GLUT_JUSTIFICATION::RIGHT, "NONE");
		if (counter == 1)
			PrintString9x15(0, y, FX_GLUT_JUSTIFICATION::RIGHT, "RECT SHADOW");
		if (counter == 2)
			PrintString9x15(0, y, FX_GLUT_JUSTIFICATION::RIGHT, "SPHERE 1 CUBE MAP");
		if (counter == 3)
			PrintString9x15(0, y, FX_GLUT_JUSTIFICATION::RIGHT, "ENVIRO CUBE MAP");
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

		const std::vector<std::string>& history = Hf::Log.getHistory();
		for (int i = 0; i < history.size(); i++) {
			std::string m = history[i];
			std::istringstream istr(m);
			std::string line;
			int j = 0;
			while (getline(istr, line)) {
				if (j == 0) {
					y += 15.0f;
					PrintString9x15(0, y, 0, "debug[%02i]: %s", i, line.c_str());
				}
				else {
					y += 15.0f;
					PrintString9x15(0, y, 0, "debug[%02i]: > %s", i, line.c_str());
				}
				j++;
			}
		}
#endif
	}

	void SSPHH_Application::RenderHelp() {
#ifdef SSPHH_RENDER_CLASSIC_OPENGL
		GLfloat xpos = 0.0f;
		GLfloat ypos = 60.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "-- Actions ----------------");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Toggle Help            [F1]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Toggle Main HUD        [F2]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Toggle HUD             [F3]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Toggle Deferred HUD    [F4]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Recompute Sky          [F5]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Output Corona SCN      [F6]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Output Corona Cube SCN [F7]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Save PBSKY PPMs       [F11]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Render Mode           [F12]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Reload Scene            [1]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Reset Scene             [2]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Increase Counter        [3]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Python Test             [4]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Python GUI              [5]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Sky Box                 [6]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "+ 1 Hour              [-/_]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "- 1 Hour              [=/+]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Rotate objects      [SPACE]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "-- Movement ----------");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Turn Up                [UP]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Turn Down            [DOWN]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Turn Left            [LEFT]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Turn Right          [RIGHT]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Forward            [W]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Backward           [S]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Left               [A]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Right              [D]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Roll Left               [Q]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Roll Right              [E]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Down               [Z]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Move Up                 [C]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "Reset camera            [R]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "-- PBSKY -------------");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "In/de-crease lat [CTRL-l/L]");
		ypos += 15.0f;
		FxGlutPrintBitmapStringJustified(xpos, ypos, screenWidth, 1, (void*)FX_GLUT_FONT::BITMAP_9_BY_15, "In/de-crease long [ALT-l/L]");
		ypos += 15.0f;
#endif
	}

	void SSPHH_Application::RenderDeferredHUD() {
		if (!Interface.showDeferredHUD)
			return;
#ifdef SSPHH_RENDER_QUAD
		Vector2i splitPoint = renderer2.GetDeferredSplitPoint();
		Recti q[4] = {
			renderer2.GetDeferredRect().GetQuadrant(Recti::UpperLeft, splitPoint),
			renderer2.GetDeferredRect().GetQuadrant(Recti::UpperRight, splitPoint),
			renderer2.GetDeferredRect().GetQuadrant(Recti::LowerLeft, splitPoint),
			renderer2.GetDeferredRect().GetQuadrant(Recti::LowerRight, splitPoint) };
		const char* names[] = {
			"Upper Left",
			"Upper Right",
			"Lower Left",
			"Lower Right" };
		for (int i = 0; i < 4; i++) {
			if (q[i].h < 15)
				break;
			float y = (float)q[i].y + 15.0f;
			float x = (float)q[i].x;
			PrintString9x15(x, y, 0, "%s (%ix%i)", names[i], q[i].w, q[i].h);
		}
#endif
	}
}