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
        {
        }

        virtual void init() override {
            mMesh->addVertexBuffer(VertexType::Position, 0, 3);
            mMesh->addVertexBuffer(VertexType::Color0, 1, 3);
        }

        virtual Mesh & getMesh() const override {
            if (mDirty && mNodes.size()) {
                std::vector<float> positions;
                std::vector<float> colors;
                positions.resize(mNodes.size() * 3);
                colors.resize(mNodes.size() * 3);
                for (unsigned i = 0; i < mNodes.size(); i++) {
                    positions[i * 3 + 0] = mNodes[i].position.x;
                    positions[i * 3 + 1] = mNodes[i].position.y;
                    positions[i * 3 + 2] = mNodes[i].position.z;
                    colors[i * 3 + 0] = mNodes[i].color.r;
                    colors[i * 3 + 1] = mNodes[i].color.g;
                    colors[i * 3 + 2] = mNodes[i].color.b;
                }
                mMesh->updateVertexBuffer(VertexType::Position, positions);
                mMesh->updateVertexBuffer(VertexType::Color0, colors);
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