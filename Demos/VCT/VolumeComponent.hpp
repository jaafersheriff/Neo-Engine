#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/ShaderBufferManager.hpp"

namespace VCT {
	START_COMPONENT(VolumeComponent);
		VolumeComponent(neo::ShaderBufferHandle handle)
			: mBufferHandle(handle)
		{}
		neo::ShaderBufferHandle mBufferHandle;
		int mDimension = 32;
		int mNodesPerVoxel = 4;
		bool mNeedsReconstruction = true;
		void imGuiEditor() override {
			ImGui::SliderInt("Dimension", &mDimension, 1, 128);
			ImGui::SliderInt("Nodes Per Voxel", &mNodesPerVoxel, 1, 8);
			mNeedsReconstruction = true;
		}
	END_COMPONENT();
}