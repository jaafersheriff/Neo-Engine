#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/ECS.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	struct LineMeshUploadedMessage;

	START_COMPONENT(LineMeshComponent);
		// TODO - replace array of structs with struct of arrays hmmm
		struct Node {
			glm::vec3 position;
			glm::vec3 color;
		};

		MeshHandle mMeshHandle;
		std::optional<glm::vec3> mOverrideColor;
		std::vector<Node> mNodes;
		bool mUseParentSpatial;
		mutable bool mDirty;

		LineMeshComponent(const MeshManager& meshManager, std::optional<glm::vec3> overrideColor = std::nullopt);
		~LineMeshComponent();

		const Mesh& getMesh(const MeshManager& meshManager, ECS::Entity self) const;
		const std::vector<Node>& getNodes() const { return mNodes; }

		void addNode(const glm::vec3 pos, glm::vec3 col = glm::vec3(1.f));
		void addNodes(const std::vector<Node>& oNodes);
		void editNode(const uint32_t i, const glm::vec3 pos, std::optional<glm::vec3> col = std::nullopt);
		void removeNode(const glm::vec3 position);
		void removeNode(const int index);
		void clearNodes();

		// Message handling
		static void registerMessageHandlers(ECS& ecs);
		static void onUploaded(ECS& ecs, const LineMeshUploadedMessage& message);

		void imGuiEditor();
	END_COMPONENT();
}
