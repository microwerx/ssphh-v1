#include <GLFW/glfw3.h>
#include <viperfish.hpp>


//////////////////////////////////////////////////////////////////////
// P R O T O T Y P E S ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace glfwt { }

// Call before GlfwTemplateInit to set parameters
void GlfwTemplateSetParameters(const std::string &windowTitle, int width, int height, int displayMode = -1);

// Call to initialize GLFW. Call after GlfwTemplateSetParameters(...)
void GlfwTemplateInit(int argc, char **argv);

// Set the Viperfish widget that is the main widget. Call after GlfwTemplateInit(...)
void GlfwTemplateWidget(Viperfish::Widget::SharedPtr widget);

// Give control over to GLFW to handle events. Call after GlfwTemplateWidget(...)
void GlfwTemplateMainLoop();