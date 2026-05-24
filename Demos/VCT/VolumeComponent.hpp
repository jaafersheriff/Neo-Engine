#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/ShaderBufferManager.hpp"

namespace VCT {
	START_COMPONENT(VolumeComponent);
		VolumeComponent() {}
		int mDimension = 32;
		int mNodesPerVoxel = 4;
		int mVoxelsPerBrick = 4;
		void imGuiEditor() override {
			ImGui::SliderPowerOfTwo("Dimension", &mDimension, 1, 1024);
			ImGui::SliderPowerOfTwo("Nodes Per Voxel", &mNodesPerVoxel, 1, 32);
			ImGui::SliderPowerOfTwo("Voxels Per Brick", &mVoxelsPerBrick, 4, 16);
		}
	END_COMPONENT();
}