#pragma once

#include "ECS/Component/Component.hpp"

#include <optional>
#include <vector>
#include <glm/glm.hpp>

namespace neo {

    class Mesh;

    class LineMeshComponent : public Component {

    public:

        struct Node {
            glm::vec3 position;
            glm::vec3 color;
        };

        std::optional<glm::vec3> mOverrideColor;
        std::vector<Node> mNodes;
        bool mWriteDepth;
        bool mUseParentSpatial;
        mutable bool mDirty;

        LineMeshComponent(GameObject* go, std::optional<glm::vec3> overrideColor = std::nullopt);

        virtual void init() override;

        const Mesh& getMesh() const;
        const std::vector<Node>& getNodes() const { return mNodes; }

        void addNode(const glm::vec3 pos, glm::vec3 col = glm::vec3(1.f));
        void addNodes(const std::vector<Node>& oNodes);
        void editNode(const uint32_t i, const glm::vec3 pos, std::optional<glm::vec3> col = std::nullopt);
        void removeNode(const glm::vec3 position);
        void removeNode(const int index);
        void clearNodes();

        virtual void imGuiEditor() override;
    private:
        Mesh* mMesh;

    };
}