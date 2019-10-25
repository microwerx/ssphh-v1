#ifndef PBSKY_VIEW_CONTROLLER_HPP
#define PBSKY_VIEW_CONTROLLER_HPP

#include <viperfish_dear_imgui.hpp>

namespace SSPHH
{
	class SSPHH_Application;
}

class PbskyViewController {
public:
	PbskyViewController(SSPHH::SSPHH_Application* app);
	~PbskyViewController();

	bool visible = true;
	float x = 64.0f;
	float y = 64.0f;
	float w = 512.0f;
	float h = 256.0f;

	bool coronaLoadEXR = true;
	std::string coronaPath;

	void init();
	void show();
private:
	SSPHH::SSPHH_Application* app_ = nullptr;
	bool firstshow = true;
};

#endif