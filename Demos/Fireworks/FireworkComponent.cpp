#include "FireworkComponent.hpp"

#include <imgui.h>

using namespace neo;

namespace Fireworks {
	FireworkComponent::FireworkComponent(const ECS::Entity& entity, const MeshManager& meshManager, uint32_t count)
		: mCount(count)
	{
		std::vector<float> emptyData;
		emptyData.resize(mCount * 4);
		MeshLoadDetails details;
		details.mPrimtive = types::mesh::Primitive::Points;
		// Position, intensity
		details.mVertexBuffers[types::mesh::VertexType::Position] = {
			4,
			0,
			types::ByteFormats::Float,
			false,
			mCount,
			0,
			static_cast<uint32_t>(sizeof(float) * 4 * mCount),
			reinterpret_cast<uint8_t*>(emptyData.data())
		};
		// Velocity, isParent
		details.mVertexBuffers[types::mesh::VertexType::Normal] = {
			4,
			0,
			types::ByteFormats::Float,
			false,
			mCount,
			0,
			static_cast<uint32_t>(sizeof(float) * 4 * mCount),
			reinterpret_cast<uint8_t*>(emptyData.data())
		};

		std::string bufferName = "FireworkBuffer_" + std::to_string(static_cast<uint32_t>(entity));
		mBuffer = meshManager.asyncLoad(HashedString(bufferName.c_str()), details);
	}

	void FireworkComponent::imGuiEditor() {
		if (ImGui::TreeNodeEx("Base", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Base Speed", &mParameters.mBaseSpeed, 0.f, 5.f);
			ImGui::SliderFloat("Velocity Decay", &mParameters.mVelocityDecay, 0.f, 1.f);
			ImGui::SliderFloat("Gravity", &mParameters.mGravity, 0.f, 10.f);
			ImGui::Checkbox("Infinite", &mParameters.mInfinite);
			ImGui::SliderFloat("Min Intensity", &mParameters.mMinIntensity, util::EP, 10.f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Parent", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Parent Speed", &mParameters.mParentSpeed , 0.f, 10.f);
			ImGui::SliderFloat("Parent Intensity Decay", &mParameters.mParentIntensityDecay, 0.f, 1.f);
			ImGui::SliderFloat("Parent Length", &mParameters.mParentLength, 0.f, 2.f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Children", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderInt("Num Children", &mParameters.mChildren, 0, mCount);
			ImGui::SliderFloat("Child Position Offset", &mParameters.mChildPositionOffset, 0.f, 0.2f);
			ImGui::SliderFloat("Child Intensity", &mParameters.mChildIntensity, 0.f, 5.f);
			ImGui::SliderFloat("Child Velocity Bias", &mParameters.mChildVelocityBias, 0.f, 1.f);
			ImGui::SliderFloat("Child Intensity Decay", &mParameters.mChildIntensityDecay, 0.f, 1.f);
			ImGui::SliderFloat("Child Length", &mParameters.mChildLength, 0.f, 2.f);
			ImGui::ColorEdit3("Child Color", &mParameters.mChildColor[0]);
			ImGui::SliderFloat("Child Color Bias", &mParameters.mChildColorBias, 0.f, 1.f);
			ImGui::TreePop();
		}
	}

}