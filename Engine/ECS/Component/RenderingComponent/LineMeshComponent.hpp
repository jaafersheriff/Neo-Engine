#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {

	struct LineMeshComponent : public Component {
		// TODO - replace array of structs with struct of arrays hmmm
		struct Node {
			glm::vec3 position;
			glm::vec3 color;
		};

		MeshHandle mMeshHandle;
		std::optional<glm::vec3> mOverrideColor;
		std::vector<Node> mNodes;
		bool mWriteDepth;
		bool mUseParentSpatial;
		mutable bool mDirty;

		LineMeshComponent(const MeshManager& meshManager, std::optional<glm::vec3> overrideColor = std::nullopt);
		~LineMeshComponent();

		const Mesh& LineMeshComponent::getMesh(const MeshManager& meshManager) const;
		const std::vector<Node>& getNodes() const { return mNodes; }
		virtual std::string getName() const override { return "LineMeshComponent"; }

		void addNode(const glm::vec3 pos, glm::vec3 col = glm::vec3(1.f));
		void addNodes(const std::vector<Node>& oNodes);
		void editNode(const uint32_t i, const glm::vec3 pos, std::optional<glm::vec3> col = std::nullopt);
		void removeNode(const glm::vec3 position);
		void removeNode(const int index);
		void clearNodes();

		virtual void imGuiEditor() override;
	};
}