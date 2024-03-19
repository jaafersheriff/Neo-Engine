#include "ECS/pch.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include <GL/glew.h>

namespace neo {

	LineMeshComponent::LineMeshComponent(MeshManager& meshManager, std::optional<glm::vec3> overrideColor) :
		mDirty(false),
		mWriteDepth(true),
		mUseParentSpatial(false),
		mOverrideColor(overrideColor)
	{

		MeshLoader::MeshBuilder builder;
		builder.mPrimtive = types::mesh::Primitive::LineStrip;
		builder.mVertexBuffers[types::mesh::VertexType::Position] = {
			3,
			0,
			types::ByteFormats::Float,
			false,
			0,
			0,
			3 * sizeof(float),
			nullptr
		};
		builder.mVertexBuffers[types::mesh::VertexType::Normal] = {
			3,
			0,
			types::ByteFormats::Float,
			false,
			0,
			0,
			3 * sizeof(float),
			nullptr
		};

		// Heh???
		HashedString random(reinterpret_cast<const char*>(this));
		mMeshHandle = meshManager.load(random, builder);
	}

	LineMeshComponent::~LineMeshComponent() {
		// mMesh->destroy();
	}

	void LineMeshComponent::addNode(const glm::vec3 pos, glm::vec3 col) {
		mNodes.push_back(Node{ pos, mOverrideColor.value_or(col) });
		mDirty = true;
	}

	void LineMeshComponent::addNodes(const std::vector<Node>& oNodes) {
		mNodes.insert(mNodes.end(), oNodes.begin(), oNodes.end());
		mDirty = true;
	}

	void LineMeshComponent::editNode(const uint32_t i, const glm::vec3 pos, std::optional<glm::vec3> col) {
		if (i < mNodes.size()) {
			mNodes[i].position = pos;
			mNodes[i].color = col.value_or(mOverrideColor.value_or(glm::vec3(1.f)));
		}
	}

	void LineMeshComponent::removeNode(const glm::vec3 position) {
		for (uint32_t i = 0; i < mNodes.size(); i++) {
			if (mNodes[i].position == position) {
				removeNode(i);
				return;
			}
		}
	}

	void LineMeshComponent::removeNode(const int index) {
		if (index >= 0 && index < (int)mNodes.size()) {
			mNodes.erase(mNodes.begin() + index);
			mDirty = true;
		}
	}

	void LineMeshComponent::clearNodes() {
		mNodes.clear();
		mDirty = true;
	}

	void LineMeshComponent::imGuiEditor() {
		if (mOverrideColor) {
			ImGui::ColorPicker3("Color", &(mOverrideColor.value())[0]);
		}
		ImGui::Separator();

		static glm::vec3 addPos(0.f);
		ImGui::Separator();
		ImGui::SliderFloat3("Add Node", &addPos[0], -50.f, 50.f);
		if (ImGui::Button("Add")) {
			addNode(addPos);
		}
		ImGui::Separator();

		static int index = 0;
		if (mNodes.size()) {
			ImGui::SliderInt("Index", &index, 0, static_cast<int>(mNodes.size() - 1));
			glm::vec3 pos = mNodes[index].position;
			glm::vec3 col = mNodes[index].color;
			bool edited = false;
			edited = edited || ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f);
			edited = edited || ImGui::ColorEdit3("Color", &col[0]);
			editNode(index, pos, col);
		}
	}
}
