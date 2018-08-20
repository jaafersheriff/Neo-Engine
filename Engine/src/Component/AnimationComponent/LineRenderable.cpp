#include "LineRenderable.hpp"

#include "Shader/LineShader.hpp"

namespace neo {

    void LineRenderable::init() {
        mesh->upload(GL_LINE_STRIP);
        addShaderType<LineShader>();
        RenderableComponent::init();
    }

    const Mesh & LineRenderable::getMesh() const {
        if (line->isDirty) {
            /* Copy vertex array */
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, line->getNodes().size() * sizeof(glm::vec3), line->getNodes().data(), GL_STATIC_DRAW));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            line->isDirty = false;
        }

        return RenderableComponent::getMesh();
    }



}