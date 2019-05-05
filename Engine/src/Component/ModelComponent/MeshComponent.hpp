#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

namespace neo {

    class MeshComponent : public Component {

    public:
        MeshComponent(GameObject* go, Mesh* m) :
            Component(go),
            mMesh(m)
        {}

        virtual const Mesh& getMesh() const { return *mMesh; }
        void replaceMesh(Mesh* m) { mMesh = m; }

    protected:
        Mesh * mMesh;
    };
}
