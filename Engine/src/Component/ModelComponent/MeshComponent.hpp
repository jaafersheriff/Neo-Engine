#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

namespace neo {

    class MeshComponent : public Component {

    public:
        MeshComponent(GameObject* go, Mesh* m) :
            Component(go),
            mMesh(m)
        {}

        virtual const Mesh& getMesh() const { return *mMesh; }
        void replaceMesh(Mesh* m) { mMesh = m; }

        virtual void imGuiEditor() override {
            auto meshes = Library::getAllMeshes();
            for (auto mesh : meshes) {
                if (ImGui::Button(mesh.first.c_str())) {
                    replaceMesh(mesh.second);
                }
            }
            ImGui::EndCombo();
        }

    protected:
        Mesh * mMesh;
    };
}
