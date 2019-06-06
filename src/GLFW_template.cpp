#include <GLFW_template.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
	std::map<int, int> keyMap;
	int specialKeyMap[256] = { 0 };
	std::vector<std::string> args;
	Viperfish::Widget::SharedPtr vfWidget = nullptr;
	bool exitMainloop = false;

	GLFWwindow* window = nullptr;
}

//////////////////////////////////////////////////////////////////////
// P R O T O T Y P E S ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace glfwt
{
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		keyMap[key] = action;
		Viperfish::SetKeyboardModifiers(
			mods & GLFW_MOD_SHIFT,
			mods & GLFW_MOD_CONTROL,
			mods & GLFW_MOD_ALT,
			mods & GLFW_MOD_SUPER,
			mods & GLFW_MOD_CAPS_LOCK,
			mods & GLFW_MOD_NUM_LOCK);
		std::string keyName = Viperfish::KeyToHTML5Name(key);
		
		HFLOGINFO("%03d %s %s",
			key,
			keyName.c_str(),
			action ? "pressed" : "released");

		if (vfWidget) {
			if (action == GLFW_PRESS)
				vfWidget->OnKeyDown(keyName, mods);
			else if (action == GLFW_RELEASE) {
				vfWidget->OnKeyUp(keyName, mods);
			}
			else if (action == GLFW_REPEAT) {
				// ... not doing repeats right now
			}
		}
		else {
			if (key == 27) {
				glfwt::exitMainloop = true;
			}
		}
	}

	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (!vfWidget) return;
		vfWidget->OnMouseMove((int)xpos, (int)ypos);
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (!vfWidget) return;

		bool pressed = (action == GLFW_PRESS);

		int mbutton = 0;

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			mbutton = VF_MOUSE_BUTTON_LEFT;
		}
		
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			mbutton = VF_MOUSE_BUTTON_RIGHT;
		}
		
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			mbutton = VF_MOUSE_BUTTON_MIDDLE;
		}

		if (pressed) {
			vfWidget->OnMouseButtonDown(mbutton);
		}
		else {
			vfWidget->OnMouseButtonUp(mbutton);
		}
	}

	// timeStamp is seconds
	void OnUpdate(double timeStamp)
	{
		if (!vfWidget) return;
		vfWidget->OnUpdate(timeStamp);
	}

	void OnRender()
	{
		if (!vfWidget) return;
		vfWidget->OnRender3D();
		vfWidget->OnRender2D();
		vfWidget->OnRenderDearImGui();
	}
}

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

	glfwSetKeyCallback(glfwt::window, glfwt::key_callback);
	glfwSetMouseButtonCallback(glfwt::window, glfwt::mouse_button_callback);
	glfwSetCursorPosCallback(glfwt::window, glfwt::cursor_position_callback);

	glfwt::exitMainloop = false;

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
	while (!glfwWindowShouldClose(glfwt::window) && !glfwt::exitMainloop) {
		double timeStamp = glfwGetTime();

		// Update widgets
		glfwt::OnUpdate(timeStamp);

		// Render widgets (3D, 2D, and then DearImGUI)
		glfwt::OnRender();

		if (!glfwt::vfWidget) {
			// Rendering
			glClearColor(1.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
		}

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
