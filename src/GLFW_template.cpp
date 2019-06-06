#include <GLFW_template.hpp>

#ifdef WIN32
#ifdef NDEBUG
#pragma comment(lib, "glfw3.lib")
#else
#pragma comment(lib, "glfw3d.lib")
#endif // NDEBUG
#endif // WIN32

//////////////////////////////////////////////////////////////////////
// G L O B A L   V A R I A B L E S ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace glfwt
{
	std::string windowTitle = "Scalable Spherical Harmonics Hierarchies (SSPHH)";
	int displayMode = GLUT_DOUBLE | GLUT_STENCIL | GLUT_RGBA | GLUT_DEPTH;
	int screenWidth = 1280;
	int screenHeight = 720;
	double Fps = 0;
	double aspectRatio = (double)glfwt::screenWidth / (double)glfwt::screenHeight;
	int mouseDX = 0;
	int mouseDY = 0;
	int mouseX = 0;
	int mouseY = 0;
	int mouseButtons = 0;
	int joystickButtonMask = 0;
	int joystickX = 0;
	int joystickY = 0;
	int joystickZ = 0;
	int joystickPollInterval = 16;
	int timer1millis = 0;
	int timer2millis = 0;
	int timer3millis = 0;
	int timer4millis = 0;
	int timer1value = 0;
	int timer2value = 0;
	int timer3value = 0;
	int timer4value = 0;
	int frameCount = 0;
	bool willResetErrorCountAfterFrame = false;
	int errorCount = 0;
	double updateInterval = 0.01666667f;
	double renderInterval = 0.01666667f;
	double Fov = 45.0;
	double zNear = 0.01;
	double zFar = 100.0;
	int keyMap[256] = { 0 };
	int specialKeyMap[256] = { 0 };
	std::vector<std::string> args;
	Viperfish::Widget::SharedPtr vfWidget = nullptr;

	GLFWwindow* window = nullptr;
}

//////////////////////////////////////////////////////////////////////
// P R O T O T Y P E S ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// M A I N   E N T R Y   P O I N T  //////////////////////////////////
//////////////////////////////////////////////////////////////////////


void GlfwTemplateSetParameters(const std::string &windowTitle, int width, int height, int displayMode)
{
	glfwt::windowTitle = windowTitle;
	glfwt::screenWidth = width;
	glfwt::screenHeight = height;
	glfwt::aspectRatio = (float)width / (float)height;
	if (displayMode >= 0) {
		glfwt::displayMode = displayMode;
	}
}

bool GlfwTemplateInit(int argc, char **argv)
{
	// Initialize GLFW
	if (!glfwInit()) {
		HFLOGERROR("GLFW cannot be initialized");
		return false;
	}

	// monitor is set to NULL
	// share is set to NULL
	glfwt::window = glfwCreateWindow(glfwt::screenWidth, glfwt::screenHeight, glfwt::windowTitle.c_str(), NULL, NULL);

	glfwMakeContextCurrent(glfwt::window);

	return true;
}

void GlfwTemplateWidget(Viperfish::Widget::SharedPtr widget)
{
	glfwt::vfWidget = widget;
}

void GlfwTemplateMainLoop()
{
	// Initialize Viperfish widgets
	if (glfwt::vfWidget) {
		glfwt::vfWidget->Init(glfwt::args);
	}
	
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(glfwt::window)) {
		// Rendering
		glClear(GL_COLOR_BUFFER_BIT);

		// Double buffering
		glfwSwapBuffers(glfwt::window);

		// Poll/process events
		glfwPollEvents();
	}

	// Kill viperfish widgets
	if (glfwt::vfWidget) {
		glfwt::vfWidget->Kill();
	}

	// Uninitialize GLFW
	glfwTerminate();
}
