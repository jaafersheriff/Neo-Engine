#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

namespace neo {

    class Library;

    class MeshComponent : public Component {

    public:
        MeshComponent(GameObject* go, Mesh* m) :
            Component(go),
            mMesh(m)
        {}

        virtual Mesh& getMesh() const { return *mMesh; }
        void replaceMesh(Mesh* m) { mMesh = m; }

        virtual void imGuiEditor() override {
            // auto meshes = Library::getAllMeshes();
            // for (auto mesh : meshes) {
            //     if (ImGui::Button(mesh.first.c_str())) {
            //         replaceMesh(mesh.second);
            //     }
            // }
        }

    protected:
        Mesh * mMesh;
    };
}
