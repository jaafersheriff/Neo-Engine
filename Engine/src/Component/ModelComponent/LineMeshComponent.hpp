#pragma once

#include "Component/AnimationComponent/LineComponent.hpp"
#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

// TODO - this should be a generic renderable component tag like the rest

namespace neo {

    namespace renderable {

        class LineMeshComponent : public MeshComponent {

        public:

            // TODO - this should happen in a system or messaging..
            LineComponent* mLine;
            glm::vec3 mColor;

            LineMeshComponent(GameObject *go, LineComponent *line, glm::vec3 color = glm::vec3(1.f)) :
                MeshComponent(go, new Mesh),
                mColor(color),
                mLine(line)
            {}

            virtual void init() override {
                mMesh->upload(GL_LINE_STRIP);
            }

            virtual const Mesh & getMesh() const override {
                if (mLine->mDirty) {
                    /* Copy vertex array */
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
                    CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mLine->getNodes().size() * sizeof(glm::vec3), mLine->getNodes().data(), GL_STATIC_DRAW));
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                    mLine->mDirty = false;
                }

                return MeshComponent::getMesh();

            }

        };
    }
}