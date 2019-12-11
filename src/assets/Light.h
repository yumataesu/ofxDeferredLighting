#pragma once

#ifndef ofxDeferredLighting_Assets_Light_h
#define ofxDeferredLighting_Assets_Light_h

#include "ofNode.h"
#include "ofxImGui.h"

namespace ofx {
namespace ofxDeferredLighting {
struct Light : public ofNode
{
	Light()
		: b_manual_(true)
		, seed_(ofRandom(0, 10))
		, radius_(ofRandom(50, 250))
		, speed_(ofRandom(0, 1))
		, diffuse_color(ofFloatColor(1.0, 1.0, 1.0, 1.0))
		, name_(std::to_string(id_) + "-")
	{
		id_++;
		float r = 30.f;
		position_offset_ = glm::vec3(ofRandom(-r, r), ofRandom(-r, r), ofRandom(0, r));
	}

	void update(const double delta_time) {
		if (!b_manual_) {
			elapsed_time_ += delta_time * speed_;
			orbitRad(PI * elapsed_time_, 0.5 + PI * elapsed_time_, radius_);
		}
	}

	void drawGui() {
		ImGui::Checkbox("Manual", &b_manual_);
		ImGui::SliderFloat(std::string(name_ + "|Spd").data(), &speed_, 0.1f, 1.0f);
		ImGui::SliderFloat(std::string(name_ + "|Rds").data(), &radius_, 1000.f, 1500.f);
		if (b_manual_) {
			if (ImGui::SliderFloat3(std::string(name_ + "|Pos").data(), &position_.x, -5000.f, 5000.f)) {
				setPosition(position_);
			}
		}
		ImGui::ColorEdit4(std::string(name_ + "|Col").data(), &diffuse_color.r);
		ImGui::Separator();
	}

	ofFloatColor diffuse_color;
	glm::vec3& getOffsetPosition() { return position_offset_; }

private:
	static int id_;
	std::string name_;
	glm::vec3 position_;
	glm::vec3 position_offset_;
	float speed_;
	float elapsed_time_;
	float seed_;
	float radius_;
	bool b_manual_;
};
}
}
#endif
