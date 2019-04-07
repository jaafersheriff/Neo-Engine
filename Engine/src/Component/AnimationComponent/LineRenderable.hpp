#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"
#include "LineComponent.hpp"

#include "GLHelper/GLHelper.hpp"

namespace neo {

    class LineRenderable : public RenderableComponent {

        public:

            LineRenderable(GameObject *go, LineComponent *line) :
                RenderableComponent(go, new Mesh),
                mLine(line)
            {}

            virtual void init() override;

            virtual const Mesh & getMesh() const override {
                if (mLine->mDirty) {
                    /* Copy vertex array */
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
                    CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mLine->getNodes().size() * sizeof(glm::vec3), mLine->getNodes().data(), GL_STATIC_DRAW));
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                    mLine->mDirty = false;
                }

                return RenderableComponent::getMesh();

            }

            LineComponent * mLine;

    };
}