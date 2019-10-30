#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

namespace neo {

    class ComputeMeshComponent : public Component {

    public:
        Mesh* mComputeMesh;
        int mNumVerts = 3;

        ComputeMeshComponent(GameObject* go) :
            Component(go)
        {
            auto buffers = Mesh::MeshBuffers{};
            buffers.vertices.push_back(-1.f);
            buffers.vertices.push_back(0.f);
            buffers.vertices.push_back(0.f);

            buffers.vertices.push_back(0.f);
            buffers.vertices.push_back(1.f);
            buffers.vertices.push_back(0.f);

            buffers.vertices.push_back(1.f);
            buffers.vertices.push_back(0.f);
            buffers.vertices.push_back(0.f);

            mComputeMesh = new Mesh(buffers);
            mComputeMesh->upload(GL_POINTS);
        }

        virtual void imGuiEditor() override {
            if (ImGui::DragInt("#Verts", &mNumVerts, 1, 0, 256)) {
                CHECK_GL(glBindVertexArray(mComputeMesh->mVAOID));
                CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mComputeMesh->mVertexBufferID));
                CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * mNumVerts * sizeof(float), 0, GL_DYNAMIC_DRAW));
                CHECK_GL(glEnableVertexAttribArray(0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mComputeMesh->mVertexBufferID));
                CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                mComputeMesh->mVertexBufferSize = mNumVerts;
            }
            if (ImGui::Button("Points")) {
                mComputeMesh->mPrimitiveType = GL_POINTS;
            }
            ImGui::SameLine();
            if (ImGui::Button("Triangles")) {
                mComputeMesh->mPrimitiveType = GL_TRIANGLES;
            }
            ImGui::SameLine();
            if (ImGui::Button("Triangle Strip")) {
                mComputeMesh->mPrimitiveType = GL_TRIANGLE_STRIP;
            }
        }
    };
}
