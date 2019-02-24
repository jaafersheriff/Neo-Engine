#include "LineRenderable.hpp"

#include "Shader/LineShader.hpp"

namespace neo {

    void LineRenderable::init() {
        mMesh->upload(GL_LINE_STRIP);
        addShaderType<LineShader>();
        RenderableComponent::init();
    }

    const Mesh & LineRenderable::getMesh() const {
        if (mLine->mDirty) {
            /* Copy vertex array */
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mMesh->mVertexBufferID));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mLine->getNodes().size() * sizeof(glm::vec3), mLine->getNodes().data(), GL_STATIC_DRAW));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            mLine->mDirty = false;
        }

        return RenderableComponent::getMesh();
    }

}