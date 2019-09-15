#pragma once

#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    namespace renderable {

        class LineMeshComponent : public MeshComponent {

        public:

            glm::vec3 mColor;

            LineMeshComponent(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
                MeshComponent(go, new Mesh),
                mColor(color)
            {}

            virtual void init() override {
                mMesh->upload(GL_LINE_STRIP);
            }

            virtual const Mesh & getMesh() const override {
                if (auto line = mGameObject->getComponentByType<LineComponent>()) {
                    if (line->mDirty) {
                        /* Copy vertex array */
                        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
                        CHECK_GL(glBufferData(GL_ARRAY_BUFFER, line->getNodes().size() * sizeof(glm::vec3), line->getNodes().data(), GL_DYNAMIC_DRAW));
                        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                        line->mDirty = false;
                    }
                }

                return MeshComponent::getMesh();

            }

        };
    }
}