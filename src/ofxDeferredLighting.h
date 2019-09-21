#pragma once

#ifndef ofxDeferredLighting_h
#define ofxDeferredLighting_h

#include "ofMain.h"
#include "ofxAutoReloadedShader.h"
#include "assets/Light.h"
#include "ofxSSAO.h"
#include "ofxBloom.h"

#define _USE_MATH_DEFINES
#include<math.h>

namespace ofx {
class DeferredLighting
{
public:
	DeferredLighting(int width, int height)
	{
		if (!lighting_shader_.load("../../../../addons/ofxDeferredLighting/assets/lighting")) {
			lighting_shader_.unload();
			lighting_shader_.load("../../addons/ofxDeferredLighting/assets/lighting");
		}

        ofFbo::Settings settings;
	    settings.width = width;
	    settings.height = height;
	    settings.useDepth = true;
	    settings.useStencil = true;
	    settings.depthStencilAsTexture = true;
	    settings.textureTarget = GL_TEXTURE_2D;
	    settings.internalformat = GL_RGBA;
	    settings.wrapModeVertical = GL_REPEAT;
	    settings.wrapModeHorizontal = GL_REPEAT;
	    settings.minFilter = GL_NEAREST;
	    settings.maxFilter = GL_NEAREST;

		int num = 2;
		lights_.reserve(num);
		for (int i = 0; i < num; ++i) {
			lights_.emplace_back(ofx::ofxDeferredLighting::Light());
		}

		quad_.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		quad_.addVertex(ofVec3f(1.0, 1.0, 0.0));
		quad_.addTexCoord(ofVec2f(1.0f, 1.0f));
		quad_.addVertex(ofVec3f(1.0, -1.0, 0.0));
		quad_.addTexCoord(ofVec2f(1.0f, 0.0f));
		quad_.addVertex(ofVec3f(-1.0, -1.0, 0.0));
		quad_.addTexCoord(ofVec2f(0.0f, 0.0f));
		quad_.addVertex(ofVec3f(-1.0, 1.0, 0.0));
		quad_.addTexCoord(ofVec2f(0.0f, 1.0f));

		ofDisableArbTex();
		ofxssao_ = std::make_unique<ofx::SSAO>(settings.width, settings.height);
		result_fbo_.allocate(settings);
		ofEnableArbTex();

		bloom_.setup(width, height, result_fbo_);
		parameter_group_.add(scale_.set("Bloom Scl", 5.0, 0.0, 10.0));
		parameter_group_.add(brightness_.set("Bloom Brightness", 6.0, 0.0, 10.0));
		parameter_group_.add(thresh_.set("Bloom Thresh", 0.5, 0.0, 10.0));
		ofDisableArbTex();
	}

	void process(
		const ofTexture& color_tex,
		const ofTexture& position_tex,
		const ofTexture& normal_tex,
		const glm::mat4& view, 
		const glm::mat4& projection,
		const glm::vec3& cam_pos)
	{

		for (auto& light : lights_) {
			light.update(ofGetLastFrameTime());
		}

		ofxssao_->process(position_tex, normal_tex, view, projection);

		result_fbo_.begin();
		lighting_shader_.begin();
		lighting_shader_.setUniformTexture("color_tex", color_tex, 0);
		lighting_shader_.setUniformTexture("position_tex", position_tex, 1);
		lighting_shader_.setUniformTexture("normal_tex", normal_tex, 2);
		lighting_shader_.setUniformTexture("ssao", ofxssao_->getTexture(), 3);
		lighting_shader_.setUniformMatrix4f("view", view);
		lighting_shader_.setUniform1i("u_light_num", lights_.size());
		lighting_shader_.setUniform3f("u_campos", cam_pos);
		for (int i = 0; i < lights_.size(); ++i) {
			lighting_shader_.setUniform4f("lights[" + to_string(i) + "].diffuse", lights_[i].diffuse_color);
			lighting_shader_.setUniform3f("lights[" + to_string(i) + "].position", lights_[i].getPosition());
		}
		quad_.draw();
		lighting_shader_.end();
		result_fbo_.end();

		bloom_.setBrightness(brightness_);
		bloom_.setScale(scale_);
		bloom_.setThreshold(thresh_);
		bloom_.process();
	}

	ofTexture& getTexture() { return bloom_.getResultFbo().getTexture(); }
	virtual void drawGui() {
		ImGui::Begin("ofxDeferredLighting");
		ofxImGui::AddGroup(parameter_group_);
		for (auto& light : lights_) light.drawGui();
		ImGui::End();
	}
private:
	ofParameterGroup parameter_group_;
	ofFbo result_fbo_;
	ofxAutoReloadedShader lighting_shader_;
	ofVboMesh quad_;

	std::unique_ptr<ofx::SSAO> ofxssao_;
	std::vector<ofx::ofxDeferredLighting::Light> lights_;

	ofxBloom bloom_;
	ofParameter<float> scale_;
	ofParameter<float> brightness_;
	ofParameter<float> thresh_;
};
}

#endif
