#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace PBR {
	struct IBLComponent : public Component {
		IBLComponent() {}

		TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		uint16_t mConvolvedCubemapResolution = 512;

		// Yikes -- these have to be mutable b/c they're set by the renderer which has a const ref to the ECS
		mutable TextureHandle mDFGLut = NEO_INVALID_HANDLE;
		mutable bool mDFGGenerated = false;
		uint16_t mDFGLutResolution = 128;

		virtual std::string getName() const override {
			return "IBLComponent";
		}

		virtual void imGuiEditor() override {
			if (ImGui::Button("Regenerate DFG Lut")) {
				mDFGGenerated = false;
			}
		};
	};
}