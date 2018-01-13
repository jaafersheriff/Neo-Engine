#include "BoundingBoxShader.hpp"

bool BoundingBoxShader::init(std::vector<Block *> *blocks) {
    /* Parent init */
    if (!blocks || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->blocks = blocks;

    addAllLocations();

    /* Create mesh */


    return true;
}

void BoundingBoxShader::addAllLocations() {
    addAttribute("vertexPos");

    addUniform("P");
    addUniform("V");
    addUniform("M");

    addUniform("size");
}

void BoundingBoxShader::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    loadP(projection);
    loadV(view);
}

void BoundingBoxShader::render(const World *world) {
    /* Bind vertices */
    glBindVertexArray(cube->vaoId);
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, cube->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

    glDisable(GL_CULL_FACE);
    for (auto &b: *blocks) {
        BoundingBox &boundingBox = b->boundingBox;

        loadM(boundingBox.m);
        loadSize(glm::distance(boundingBox.worldMax, boundingBox.worldMin));

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, cube->vertBuf.size() / 3);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

;
    }
    /* Unbind */
    glDisableVertexAttribArray(getAttribute("vertexPos"));
    Shader::unloadMesh();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void BoundingBoxShader::cleanUp() {
    Shader::cleanUp();
}

void BoundingBoxShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void BoundingBoxShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void BoundingBoxShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void BoundingBoxShader::loadSize(const float scale) {
    this->loadFloat(getUniform("size"), scale);
}