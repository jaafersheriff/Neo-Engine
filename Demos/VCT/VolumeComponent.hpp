#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/ShaderBufferManager.hpp"

namespace VCT {
	START_COMPONENT(VolumeComponent);
		VolumeComponent() {}
		uint16_t mDimension = 256;
		uint16_t mVoxelsPerBrick = 2;
		uint16_t mMaxBricks = 1024;
		void imGuiEditor() override {
			ImGui::SliderPowerOfTwo("Dimension", &mDimension, 1, 8192);
			ImGui::SliderPowerOfTwo("Voxels Per Brick", &mVoxelsPerBrick, 2, 8);
			ImGui::SliderPowerOfTwo("Max Bricks", &mMaxBricks, mDimension, 4096);
			ImGui::Text("Logical bricks per Axis: %d", static_cast<int>(getLogicalBricksPerAxis()));
			ImGui::Text("Physical bricks per Axis: %d", static_cast<int>(getPhysicalBricksPerAxis()));
		}

		uint16_t getLogicalBricksPerAxis() const {
			return static_cast<uint16_t>(std::ceil(mDimension / static_cast<float>(mVoxelsPerBrick)));
		}

		uint16_t getPhysicalBricksPerAxis() const {
			return static_cast<uint16_t>(std::ceil(std::cbrt(mMaxBricks)));
		}

	END_COMPONENT();
}