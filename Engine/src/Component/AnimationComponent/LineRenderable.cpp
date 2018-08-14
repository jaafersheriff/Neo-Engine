#include "LineRenderable.hpp"

#include "Shader/LineShader.hpp"

namespace neo {

    void LineRenderable::init() {
        CHECK_GL(glGenVertexArrays(1, (GLuint *)&mesh->vaoId));
        CHECK_GL(glBindVertexArray(mesh->vaoId));
        CHECK_GL(glGenBuffers(1, (GLuint *)&mesh->vertBufId));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
        CHECK_GL(glEnableVertexAttribArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
        CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
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