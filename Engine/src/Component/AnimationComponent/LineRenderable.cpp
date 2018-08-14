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
        if (isDirty) {
            /* Copy vertex array */
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, nodes.size() * sizeof(glm::vec3), nodes.data(), GL_STATIC_DRAW));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            isDirty = false;
        }

        return RenderableComponent::getMesh();
    }

    void LineRenderable::addNode(const glm::vec3 &node) {
        nodes.push_back(node);
        isDirty = true;
    }

    void LineRenderable::addNodes(const std::vector<glm::vec3> &oNodes) {
        nodes.insert(nodes.end(), oNodes.begin(), oNodes.end());
        isDirty = true;
    }

    void LineRenderable::removeNode(const glm::vec3 &node) {
        auto it = std::find(nodes.begin(), nodes.end(), node);
        if (it != nodes.end()) {
            nodes.erase(it);
            isDirty = true;
        }
    }

    void LineRenderable::removeNode(const int index) {
        if (index >= 0 && index < (int)nodes.size()) {
            nodes.erase(nodes.begin() + index);
            isDirty = true;
        }
    }

}