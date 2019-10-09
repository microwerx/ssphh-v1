#include <ssphhapp.hpp>

namespace SSPHH
{
	void SSPHH_Application::RenderFixedFunctionGL() {
#ifdef SSPHH_RENDER_CLASSIC_OPENGL
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
		// gluPerspective(ssg.camera.fov, aspect, 0.01, 200.0);
		Matrix4f perspective;
		perspective.PerspectiveY(ssg.camera.fov, aspect, 0.01f, 200.0f);
		glMultMatrixf(perspective.const_ptr());
		glMultMatrixf((Interface.preCameraMatrix).AsInverse().const_ptr());
		//glMultMatrixf((ssg.camera.viewMatrix).const_ptr());

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

		// Render Sun and Environment

		// FxDrawGL1EnvironmentCube(50.0f, enviroCubeTexture1.GetTextureId());
		FxDrawGL1EnvironmentCube(50.0f, PBSkyCubeMap.GetTextureId());

		// Render Scene

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);

		if (ssg.geometryObjects.size() == 0)
			FxDrawGL1SolidTeapot(1.0);

		glDisable(GL_LIGHTING);

		// Camera
		glPushMatrix();
		glMultMatrixf(ssg.camera.viewMatrix.AsInverse().const_ptr());
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, -1.0f);
		FxDrawGL1WireCone(2 * sin(ssg.camera.fov * FX_DEGREES_TO_RADIANS * 0.5), 1.0, 32, 2);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.5f);
		FxDrawGL1WireCube(1.0);
		glPopMatrix();
		FxDrawGL1SixAxis(1.0);
		FxDrawGL1WireFrustumf(ssg.camera.projectionMatrix.const_ptr());
		glPopMatrix();

		// Scene center
		FxDrawGL1SixAxis(2.0);

		// Spheres
		for (auto s = ssg.spheres.begin(); s != ssg.spheres.end(); s++) {
			glPushMatrix();
			glMultMatrixf(s->second.transform.const_ptr());
			FxDrawGL1SolidSphere(0.5f, 16, 16);
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
			FxDrawGL1WireCube(ssg.geometryObjects[g->second.objectId].BoundingBox.MaxSize());
			glPopMatrix();

			glEnable(GL_LIGHTING);
			OBJStaticModel& osm = ssg.geometryObjects[g->second.objectId];
			glEnable(GL_TEXTURE_2D);
			osm.RenderGL1();
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}

		glPushMatrix();
		sunVector *= 20.0f;
		glTranslatef(sunVector.x, sunVector.y, sunVector.z);
		FxDrawGL1SolidSphere(0.5f, 16, 16);
		glPopMatrix();

		// Point Lights
		glEnable(GL_BLEND);
		for (auto pl = ssg.pointLights.begin(); pl != ssg.pointLights.end(); pl++) {
			glPushMatrix();
			glTranslatef(pl->position.x, pl->position.y, pl->position.z);
			glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
			FxDrawGL1SolidSphere(0.5f, 16, 16);
			glColor4f(1.0f, 1.0f, 0.0f, 0.1f);
			FxDrawGL1SolidSphere(pl->falloffRadius, 16, 16);
			glPopMatrix();
		}
		glDisable(GL_BLEND);

		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
#endif
	}

	void SSPHH_Application::RenderGLES20() {
		RenderFixedFunctionGL();
		FxSetErrorMessage(__FILE__, __LINE__, "%s: Part 1", __FUNCTION__);
		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);
		FxSetDefaultErrorMessage();
	}
}