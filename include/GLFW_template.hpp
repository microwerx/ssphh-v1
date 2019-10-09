#ifndef GLFW_TEMPLATE_HPP
#define GLFW_TEMPLATE_HPP

#include <viperfish.hpp>

//////////////////////////////////////////////////////////////////////
// P R O T O T Y P E S ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace glfwt { }

// Call before GlfwTemplateInit to set parameters
void GlfwTemplateSetParameters(const std::string &windowTitle, int width, int height);

// Call to initialize GLFW. Call after GlfwTemplateSetParameters(...)
bool GlfwTemplateInit(int argc, char **argv);

// Set the Viperfish widget that is the main widget. Call after GlfwTemplateInit(...)
void GlfwTemplateWidget(Vf::Widget::SharedPtr widget);

// Give control over to GLFW to handle events. Call after GlfwTemplateWidget(...)
void GlfwTemplateMainLoop();

#endif