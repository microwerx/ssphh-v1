#include <string>
#include <vector>
#include <fluxions.hpp>
#include <GLUT_template.hpp>
#include <GL/freeglut.h>

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib, "freeglut_staticd.lib")
#else
#pragma comment(lib, "freeglut_static.lib")
#endif // _DEBUG
#endif // _WIN32

//////////////////////////////////////////////////////////////////////
// G L O B A L   V A R I A B L E S ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

std::string gt_windowTitle = "Scalable Spherical Harmonics Hierarchies (SSPHH)";
#ifdef _WIN32
int gt_displayMode = GLUT_DOUBLE | GLUT_STENCIL | GLUT_RGBA | GLUT_DEPTH; // | GLUT_SRGB; // | GLUT_BORDERLESS;
#else
int gt_displayMode = GLUT_DOUBLE | GLUT_STENCIL | GLUT_RGBA | GLUT_DEPTH;
#endif
int gt_screenWidth = 1280;
int gt_screenHeight = 720;
double gt_Fps = 0;
double gt_aspectRatio = (double)gt_screenWidth / (double)gt_screenHeight;
int gt_mouseDX = 0;
int gt_mouseDY = 0;
int gt_mouseX = 0;
int gt_mouseY = 0;
int gt_mouseButtons = 0;
int gt_joystickButtonMask = 0;
int gt_joystickX = 0;
int gt_joystickY = 0;
int gt_joystickZ = 0;
int gt_joystickPollInterval = 16;
int gt_timer1millis = 0;
int gt_timer2millis = 0;
int gt_timer3millis = 0;
int gt_timer4millis = 0;
int gt_timer1value = 0;
int gt_timer2value = 0;
int gt_timer3value = 0;
int gt_timer4value = 0;
int gt_frameCount = 0;
bool gt_willResetErrorCountAfterFrame = false;
int gt_errorCount = 0;
double gt_updateInterval = 0.01666667f;
double gt_renderInterval = 0.01666667f;
double gt_Fov = 45.0;
double gt_zNear = 0.01;
double gt_zFar = 100.0;
int gt_keyMap[256] = { 0 };
int gt_specialKeyMap[256] = { 0 };
std::vector<std::string> gt_args;
Vf::Widget::SharedPtr gt_Widget = nullptr;

//////////////////////////////////////////////////////////////////////
// P R O T O T Y P E S ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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

void OnRender3D();
void OnRender2D();
void Set2DViewport();
void Set3DViewport();
void RenderOpenGL11TestGui();
void RenderCheckerboard();
void RenderOpenGL11TestScene();

//////////////////////////////////////////////////////////////////////
// M A I N   E N T R Y   P O I N T  //////////////////////////////////
//////////////////////////////////////////////////////////////////////

void GlutTemplateSetParameters(const std::string &windowTitle, int width, int height, int displayMode)
{
	gt_windowTitle = windowTitle;
	gt_screenWidth = width;
	gt_screenHeight = height;
	gt_aspectRatio = (float)width / (float)height;
	if (displayMode >= 0) {
		gt_displayMode = displayMode;
	}
}

void GlutTemplateInit(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		gt_args.push_back(argv[i]);
	}

#ifdef _WIN32
	SetProcessDPIAware();
#endif

	glutInit(&argc, argv);
	glutInitDisplayMode(gt_displayMode);
	if (glutGet(GLUT_DISPLAY_MODE_POSSIBLE) == 0) {
		HFLOGERROR("GLUT Display Mode not supported");
		exit(-1);
	}
#ifdef __unix__
	glutInitContextVersion(4, 0);
#endif
#ifdef _WIN32
	glutInitContextVersion(4, 0);
#endif
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitWindowSize(gt_screenWidth, gt_screenHeight);
	glutCreateWindow(gt_windowTitle.c_str());
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
	if (gt_timer1millis != 0)
		glutTimerFunc(gt_timer1millis, timer1, gt_timer1value);
	if (gt_timer2millis != 0)
		glutTimerFunc(gt_timer2millis, timer2, gt_timer2value);
	if (gt_timer3millis != 0)
		glutTimerFunc(gt_timer3millis, timer3, gt_timer3value);
	if (gt_timer4millis != 0)
		glutTimerFunc(gt_timer4millis, timer4, gt_timer4value);
	glutMenuStateFunc(menu);
	glutMenuStatusFunc(menustatus);
	glutWindowStatusFunc(windowstatus);
	glutJoystickFunc(joystick, gt_joystickPollInterval);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
}

void GlutTemplateWidget(Vf::Widget::SharedPtr widget)
{
	gt_Widget = widget;
}

void GlutTemplateMainLoop()
{
	if (gt_Widget)
	{
		gt_Widget->Init(gt_args);
	}
	glutMainLoop();
	if (gt_Widget)
	{
		gt_Widget->Kill();
	}
}

//////////////////////////////////////////////////////////////////////
// G L U T   C A L L B A C K S ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void display()
{
	if (gt_Widget) {
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnPreRender();
		w->OnRender3D();
		w->OnRender2D();
		w->OnRenderDearImGui();
		w->OnPostRender();
	}
	else
	{
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glViewport(0, 0, gt_screenWidth, gt_screenHeight);
		glLoadIdentity();
		Set3DViewport();
		OnRender3D();

		glViewport(0, 0, gt_screenWidth, gt_screenHeight);
		glLoadIdentity();
		Set2DViewport();
		OnRender2D();
	}

	if (gt_displayMode & GLUT_DOUBLE)
	{
		glutSwapBuffers();
	}
	gt_frameCount++;
	if (gt_willResetErrorCountAfterFrame)
		gt_errorCount = 0;
}

void reshape(int width, int height)
{
	gt_aspectRatio = (double)width / (double)height;
	gt_screenWidth = width;
	gt_screenHeight = height;

	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnReshape(width, height);
	}
	else
	{
		glViewport(0, 0, width, height);
	}
}

void keyboard(unsigned char key, int x, int y)
{
	gt_keyMap[key] = 1;
	std::string keyName = Vf::KeyToHTML5Name(key);

	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnKeyDown(keyName, glutGetModifiers());
	}
	else
	{
		if (key == 27)
			glutLeaveMainLoop();
	}
}

void keyboardup(unsigned char key, int x, int y)
{
	gt_keyMap[key] = 0;

	std::string keyName = Vf::KeyToHTML5Name(key);
	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnKeyUp(keyName, glutGetModifiers());
	}
}

void special(int key, int x, int y)
{
	gt_specialKeyMap[key] = 1;

	std::string keyName = Vf::SpecialKeyToHTML5Name(key);
	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnKeyDown(keyName, glutGetModifiers());
	}
}

void specialup(int key, int x, int y)
{
	gt_specialKeyMap[key] = 0;

	std::string keyName = Vf::SpecialKeyToHTML5Name(key);
	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnKeyUp(keyName, glutGetModifiers());
	}
}

void mouse(int button, int state, int x, int y)
{
	int buttonMask = 1 << button;
	if (state == GLUT_UP)
	{
		gt_mouseButtons &= ~buttonMask;
	}
	else
	{
		gt_mouseButtons |= buttonMask;
	}
	gt_mouseDX = x - gt_mouseX;
	gt_mouseDY = x - gt_mouseY;
	gt_mouseX = x;
	gt_mouseY = y;

	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnMouseMove(gt_mouseX, gt_mouseY);
	}
	if (state == GLUT_DOWN)
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnMouseButtonDown(button);
		}
	}
	else if (state == GLUT_UP) {
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnMouseButtonUp(button);
		}
	}
}

void motion(int x, int y)
{
	gt_mouseDX = x - gt_mouseX;
	gt_mouseDY = x - gt_mouseY;
	gt_mouseX = x;
	gt_mouseY = y;

	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnMouseMove(gt_mouseX, gt_mouseY);
	}
}

void passive(int x, int y)
{
	gt_mouseDX = x - gt_mouseX;
	gt_mouseDY = x - gt_mouseY;
	gt_mouseX = x;
	gt_mouseY = y;

	if (gt_Widget)
	{
		using Vf::Widget;
		Widget::SharedPtr w = gt_Widget;
		w->OnMouseMove(x, y);
	}
}

void entry(int state)
{
	if (state == GLUT_ENTERED)
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnMouseEnter();
		}
	}
	else
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnMouseLeave();
		}
	}
}

void visibility(int state)
{
	if (state == GLUT_VISIBLE)
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnWindowVisible();
		}
	}
	else if (state == GLUT_NOT_VISIBLE)
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnWindowHidden();
		}
	}
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

	gt_Fps = 20.0 / (fps1 - fps0);

	if (updatedt > gt_updateInterval)
	{
		if (gt_Widget)
		{
			using Vf::Widget;
			Widget::SharedPtr w = gt_Widget;
			w->OnUpdate(updatedt);
		}
		updatedt = 0;
	}

	if (renderdt > gt_renderInterval)
	{
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
	gt_joystickButtonMask = buttonMask;
	gt_joystickX = x;
	gt_joystickY = y;
	gt_joystickZ = z;
}

//////////////////////////////////////////////////////////////////////
// F O N T   F U N C T I O N S ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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
		glRasterPos2f((GLfloat)(gt_screenWidth - pixelWidthOfString), (GLfloat)y);
	}
	else if (justification == CENTER)
	{
		glRasterPos2f((GLfloat)(gt_screenWidth - pixelWidthOfString) / 2.0f, (GLfloat)y);
	}

	FxGlutBitmapString(font, buffer);
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
		glRasterPos2f((float)(gt_screenWidth - pixelWidthOfString), (float)y);
	}
	else if (justification == CENTER)
	{
		glRasterPos2f((float)(gt_screenWidth - pixelWidthOfString) / 2.0f, (float)y);
	}

	FxGlutStrokeString(font, buffer);
}

// Default GLUT Display Actions //

void OnRender3D()
{
	RenderOpenGL11TestGui();
}

void OnRender2D()
{
	RenderOpenGL11TestScene();
}

void Set2DViewport()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Fluxions::Matrix4f ortho2d;
	ortho2d.Ortho2D(0.0f, (float)gt_screenWidth, (float)gt_screenHeight, 0.0f);
	glMultMatrixf(ortho2d.const_ptr());
	glMatrixMode(GL_MODELVIEW);
}

void Set3DViewport()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Fluxions::Matrix4f perspective;
	perspective.PerspectiveY((float)gt_Fov, (float)gt_aspectRatio, (float)gt_zNear, (float)gt_zFar);
	glMultMatrixf(perspective.const_ptr());
	glMatrixMode(GL_MODELVIEW);
}

void RenderOpenGL11TestGui()
{
	glColor3f(1.0f, 1.0f, 1.0f);
	FxGlutPrintString9x15(0.0, 0.0, gt_screenWidth, 0, "%s", gt_windowTitle.c_str());
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

	Fluxions::Matrix4f camera = Fluxions::Matrix4f::MakeLookAt(
		Fluxions::Vector3f(0.0f, 0.0f, 6.0f),
		Fluxions::Vector3f(0.0f, 0.0f, 0.0f),
		Fluxions::Vector3f(0.0f, 1.0f, 0.0f)
	);
	glLoadMatrixf(camera.const_ptr());
	//glTranslatef(0, 0, -20 - g_distance);
	//glRotatef(yrot, 0.0, 1.0, 0.0);
	glutSolidTeapot(1.0);
	glPopMatrix();

	glColor3f(1, 0, 0);
	glLoadIdentity();
	//glTranslatef(0, -4, -20 - g_distance);
	glLoadMatrixf(camera.const_ptr());
	glTranslatef(0, -4, 0);
	glRotatef(-90, 1, 0, 0);
	glRectf(-1, -1, 1, 1);

	RenderCheckerboard();

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
}
