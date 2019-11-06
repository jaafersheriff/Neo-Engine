#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class MetaballsMeshComponent : public Component {

public:
    Mesh* mMesh;
    int mNumVerts = 256;

    MetaballsMeshComponent(GameObject* go) :
        Component(go)
    {
        MeshBuffers buffers;
        buffers.vertices.resize(mNumVerts * 4);
        mMesh = new Mesh(buffers, GL_TRIANGLE_STRIP);

        // reUploadMesh();
    }

    virtual void imGuiEditor() override {
        if (ImGui::DragInt("#Verts", &mNumVerts, 1, 0, 2048)) {
            mMesh->mBuffers.vertices.resize(mNumVerts * (3 + 1));
            mMesh->mBuffers.normals.resize(mNumVerts * 3);
            mMesh->mBuffers.texCoords.resize(mNumVerts * 2);
            // mMesh->mBuffers.indices.resize(mNumVerts * 6);
            mMesh->upload();
            // reUploadMesh();
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

    void reUploadMesh() {
        CHECK_GL(glBindVertexArray(mMesh->mVAOID));
        {
            CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMesh->mVertexBufferID));
            if (mNumVerts) {
                CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, mMesh->mBuffers.vertices.size() * sizeof(float), &mMesh->mBuffers.vertices[0], GL_DYNAMIC_DRAW));
            }
            CHECK_GL(glEnableVertexAttribArray(0));
            CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMesh->mVertexBufferID));
            CHECK_GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
        // {
        //     CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMesh->mNormalBufferID));
        //     if (mNumVerts) {
        //         CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, mMesh->mBuffers.normals.size() * sizeof(float), &mMesh->mBuffers.normals[0], GL_DYNAMIC_DRAW));
        //     }
        //     CHECK_GL(glEnableVertexAttribArray(1));
        //     CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mNormalBufferID));
        //     CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        // }
        // {
        //     CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMesh->mTexBufferID));
        //     if (mNumVerts) {
        //         CHECK_GL(glBufferData(GL_SHADER_STORAGE_BUFFER, mMesh->mBuffers.indices.size() * sizeof(float), &mMesh->mBuffers.texCoords[0], GL_DYNAMIC_DRAW));
        //     }
        //     CHECK_GL(glEnableVertexAttribArray(2));
        //     CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mTexBufferID));
        //     CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        // }
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
};
