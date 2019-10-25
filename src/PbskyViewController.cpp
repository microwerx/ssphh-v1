#include <PbskyViewController.hpp>
#include <ssphhapp.hpp>

PbskyViewController::PbskyViewController(SSPHH::SSPHH_Application* app) :
	app_(app) {}

PbskyViewController::~PbskyViewController() {}

void PbskyViewController::init() {
	ImGui::SetNextWindowPos(ImVec2(app_->screenWidth - w, app_->screenHeight - h));
	ImGui::SetNextWindowSize(ImVec2(w, h));
	firstshow = false;
}
void PbskyViewController::show() {
	if (!visible) return;
	if (firstshow) init();

	ImGui::SetNextWindowContentWidth(w);
	ImGui::Begin("PBSKY");
	if (ImGui::Button("Hide")) {
		visible = false;
	}
	
	ImGui::Text("Hosek-Wilkie");
	ImGui::SameLine();
	if (ImGui::Button("Gen")) {
		app_->RegenHosekWilkieSky();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		app_->SaveHosekWilkieSky();
	}
	
	ImGui::Text("Corona");
	ImGui::SameLine();
	if (ImGui::Button("Gen")) {
		app_->RegenCoronaSky();
	}
	if (ImGui::Button("Load")) {
		app_->LoadCoronaSky(coronaLoadEXR, coronaPath);
	}

	ImGui::End();
}