#pragma once

#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

#include <optional>

namespace neo {

    class LineMeshComponent : public MeshComponent {

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

        LineMeshComponent(GameObject *go, std::optional<glm::vec3> overrideColor = std::nullopt) :
            MeshComponent(go, new Mesh(GL_LINE_STRIP)),
            mDirty(false),
            mWriteDepth(true),
            mUseParentSpatial(false),
            mOverrideColor(overrideColor)
        {}

        virtual void init() override {
            mMesh->upload();

            // create color buffer
            CHECK_GL(glBindVertexArray(mMesh->mVAOID));
            CHECK_GL(glGenBuffers(1, (GLuint *)&mMesh->mNormalBufferID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mNormalBufferID));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mNormalBufferID));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        virtual Mesh & getMesh() const override {
            if (mDirty && mNodes.size()) {
                std::vector<glm::vec3> positions;
                std::vector<glm::vec3> colors;
                for (Node n : mNodes) {
                    positions.push_back(n.position);
                    colors.push_back(n.color);
                }
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
                CHECK_GL(glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mNormalBufferID));
                CHECK_GL(glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_DYNAMIC_DRAW));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                mDirty = false;
            }

            return MeshComponent::getMesh();

        }

        const std::vector<Node>& getNodes() const { return mNodes; }

        void addNode(const glm::vec3 pos, glm::vec3 col = glm::vec3(1.f)) {
            mNodes.push_back(Node{ pos, mOverrideColor.value_or(col) });
            mDirty = true;
        }

        void addNodes(const std::vector<Node> &oNodes) {
            mNodes.insert(mNodes.end(), oNodes.begin(), oNodes.end());
            mDirty = true;
        }

        void editNode(const unsigned i, const glm::vec3 pos, std::optional<glm::vec3> col = std::nullopt) {
            if (i < mNodes.size()) {
                mNodes[i].position = pos;
                mNodes[i].color = col.value_or(mOverrideColor.value_or(glm::vec3(1.f)));
            }
        }

        void removeNode(const glm::vec3 position) {
            for (unsigned i = 0; i < mNodes.size(); i++) {
                if (mNodes[i].position == position) {
                    removeNode(i);
                    return;
                }
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

        virtual void imGuiEditor() override {
            if (mOverrideColor) {
                ImGui::SliderFloat("Color", &(mOverrideColor.value())[0], 0.f, 1.f);
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
                ImGui::SliderInt("Index", &index, 0, mNodes.size() - 1);
                glm::vec3 pos = mNodes[index].position;
                glm::vec3 col = mNodes[index].color;
                bool edited = false;
                edited = edited || ImGui::SliderFloat3("Position", &pos[0], -25.f, 25.f);
                edited = edited || ImGui::SliderFloat3("Color", &col[0], 0.f, 1.f);
                editNode(index, pos, col);
            }
        }

    };
}