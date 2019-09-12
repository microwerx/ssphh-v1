#ifndef GLUT_TEMPLATE_HPP
#define GLUT_TEMPLATE_HPP

#include <viperfish.hpp>

void GlutTemplateSetParameters(const std::string &windowTitle, int width, int height, int displayMode = -1);
void GlutTemplateInit(int argc, char **argv);
void GlutTemplateWidget(Vf::Widget::SharedPtr widget);
void GlutTemplateMainLoop();

void PrintString9x15(float x, float y, int justification, const char *format, ...);
void PrintBitmapStringJustified(float x, float y, int justification, void *font, const char *format, ...);
void PrintStrokeStringJustified(float x, float y, int justification, void *font, const char *format, ...);

extern Vf::Widget::SharedPtr gt_Widget;

#endif
