#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/ShaderBufferManager.hpp"

namespace VCT {
	START_COMPONENT(VolumeComponent);
		VolumeComponent() {}
		int mDimension = 32;
		int mNodesPerVoxel = 4;
		void imGuiEditor() override {
			ImGui::SliderInt("Dimension", &mDimension, 1, 128);
			ImGui::SliderInt("Nodes Per Voxel", &mNodesPerVoxel, 1, 8);
		}
	END_COMPONENT();
}