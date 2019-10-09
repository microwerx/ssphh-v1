#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace SSPHH
{
	void SSPHH_Application::OnUpdate(double deltaTime) {
		Widget::OnUpdate(deltaTime);

		Hf::StopWatch stopwatch;
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

		if (Interface.tools.showScenegraphEditor) {
			ProcessScenegraphTasks();
		}

		if (Interface.tools.showTestsWindow) {
			Test();
		}

		GI_ScatterJobs();
		GI_GatherJobs();
	}

	void SSPHH_Application::DoInterfaceUpdate(double deltaTime) {
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
			}
			else {
				gpMoveRate = 1.0;
			}
			if (gamepads[0].rthumbAmount() > 0.0f) {
				gpTurnRate = gamepads[0].rthumbAmount();
			}
			else {
				gpTurnRate = 1.0;
			}
		}

		double mdx = mouse.buttons[0] ? mouse.dragStates[0].currentDelta.x : 0.0;
		double mdy = mouse.buttons[0] ? mouse.dragStates[0].currentDelta.y : 0.0;

		Vector2f dimensions = Vector2f((float)renderer2.GetDeferredRect().w, (float)renderer2.GetDeferredRect().h);
		double rectLength = dimensions.length();
		double scale = -180.0 / rectLength;
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
		rot.Rotate((float)Interface.turnX, 1.0f, 0.0f, 0.0f);
		rot.Rotate((float)Interface.turnY, 0.0f, 1.0f, 0.0f);
		rot.Rotate((float)Interface.turnZ, 0.0f, 0.0f, 1.0f);

		Matrix4f trans;
		trans.Translate((float)Interface.moveX, (float)Interface.moveY, (float)Interface.moveZ);

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
}