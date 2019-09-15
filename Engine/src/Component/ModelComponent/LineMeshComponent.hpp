#pragma once

#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    class LineMeshComponent : public MeshComponent {

    public:

        glm::vec3 mColor;
        std::vector<glm::vec3> mNodes;
        mutable bool mDirty;

        LineMeshComponent(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
            MeshComponent(go, new Mesh),
            mColor(color),
            mDirty(false)
        {}

        virtual void init() override {
            mMesh->upload(GL_LINE_STRIP);
        }

        virtual const Mesh & getMesh() const override {
            if (mDirty) {
                /* Copy vertex array */
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
                CHECK_GL(glBufferData(GL_ARRAY_BUFFER, getNodes().size() * sizeof(glm::vec3), getNodes().data(), GL_DYNAMIC_DRAW));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                mDirty = false;
            }

            return MeshComponent::getMesh();

        }

        const std::vector<glm::vec3>& getNodes() const { return mNodes; }

        void addNode(const glm::vec3 node) {
            mNodes.push_back(node);
            mDirty = true;
        }

        void addNodes(const std::vector<glm::vec3> &oNodes) {
            mNodes.insert(mNodes.end(), oNodes.begin(), oNodes.end());
            mDirty = true;
        }

        void removeNode(const glm::vec3 node) {
            auto it = std::find(mNodes.begin(), mNodes.end(), node);
            if (it != mNodes.end()) {
                mNodes.erase(it);
                mDirty = true;
            }
        }

        void removeNode(const int index) {
            if (index >= 0 && index < (int)mNodes.size()) {
                mNodes.erase(mNodes.begin() + index);
                mDirty = true;
            }
        }

        void clearNodes() {
            mNodes.clear();
            mDirty = true;
        }

    };
}