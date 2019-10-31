#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

namespace neo {

    class MetaballsMeshComponent : public Component {

    public:
        Mesh* mMesh;
        int mNumVerts = 256;

        MetaballsMeshComponent(GameObject* go) :
            Component(go)
        {
            mMesh = new Mesh();
            mMesh->upload(GL_POINTS);

            _compute();
        }

        virtual void imGuiEditor() override {
            if (ImGui::DragInt("#Verts", &mNumVerts, 1, 0, 256)) {
                _compute();
            }
            if (ImGui::Button("Points")) {
                mMesh->mPrimitiveType = GL_POINTS;
            }
            ImGui::SameLine();
            if (ImGui::Button("Triangles")) {
                mMesh->mPrimitiveType = GL_TRIANGLES;
            }
            ImGui::SameLine();
            if (ImGui::Button("Triangle Strip")) {
                mMesh->mPrimitiveType = GL_TRIANGLE_STRIP;
            }
        }

    private:
        void _compute() {
            CHECK_GL(glBindVertexArray(mMesh->mVAOID));
            CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMesh->mVertexBufferID));
            CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * mNumVerts * sizeof(float), 0, GL_DYNAMIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            mMesh->mVertexBufferSize = mNumVerts;
        }
    };
}
