#include "ECS/pch.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include <GL/glew.h>

namespace neo {

	LineMeshComponent::LineMeshComponent(const MeshResourceManager& meshManager, std::optional<glm::vec3> overrideColor) :
		mDirty(false),
		mWriteDepth(true),
		mUseParentSpatial(false),
		mOverrideColor(overrideColor)
	{

		MeshLoadDetails builder;
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
		MeshHandle random(static_cast<uint32_t>(reinterpret_cast<std::uintptr_t>(this)));
		mMeshHandle = meshManager.asyncLoad(random, std::move(builder));
	}

	LineMeshComponent::~LineMeshComponent() {
		// TODO - destroy meshhandle
	}

	const Mesh& LineMeshComponent::getMesh(const MeshResourceManager& meshManager) const {
		if (!meshManager.isValid(mMeshHandle)) {
			return meshManager.resolve(NEO_INVALID_HANDLE);
		}

		auto& mesh = meshManager.resolve(mMeshHandle);
		if (mDirty && mNodes.size()) {
			TRACY_ZONE();
			std::vector<float> positions;
			std::vector<float> colors;
			positions.resize(mNodes.size() * 3);
			colors.resize(mNodes.size() * 3);
			for (uint32_t i = 0; i < mNodes.size(); i++) {
				positions[i * 3 + 0] = mNodes[i].position.x;
				positions[i * 3 + 1] = mNodes[i].position.y;
				positions[i * 3 + 2] = mNodes[i].position.z;
				colors[i * 3 + 0] = mNodes[i].color.r;
				colors[i * 3 + 1] = mNodes[i].color.g;
				colors[i * 3 + 2] = mNodes[i].color.b;
			}

			meshManager.transact(mMeshHandle, [positions, colors](Mesh& mesh) {
				mesh.updateVertexBuffer(
					types::mesh::VertexType::Position,
					static_cast<uint32_t>(positions.size()),
					static_cast<uint32_t>(positions.size() * sizeof(float)),
					reinterpret_cast<uint8_t*>(const_cast<float*>(positions.data()))
				);
				mesh.updateVertexBuffer(
					types::mesh::VertexType::Normal,
					static_cast<uint32_t>(colors.size()),
					static_cast<uint32_t>(colors.size() * sizeof(float)),
					reinterpret_cast<uint8_t*>(const_cast<float*>(colors.data()))
				);
				});

			mDirty = false;
		}
		return mesh;
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
			mDirty = true;
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
			mDirty |= ImGui::ColorPicker3("Color", &(mOverrideColor.value())[0]);
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
			if (edited) {
				editNode(index, pos, col);
			}
		}
	}
}
