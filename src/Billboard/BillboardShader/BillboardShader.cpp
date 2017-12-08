#include "BillboardShader.hpp"

bool BillboardShader::init() {
    if (!Shader::init()) {
        return false;
    }

    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("cameraPosition");

    /* Billboard things */
    addUniform("center");
    addUniform("size");

    /* Texture */
    addUniform("textureImage");

    return true;
}

void BillboardShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void BillboardShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void BillboardShader::loadCameraPosition(const glm::vec3 c) {
    this->loadVec3(getUniform("cameraPosition"), c);
}

void BillboardShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void BillboardShader::loadCenter(const glm::vec3 c) {
    this->loadVec3(getUniform("center"), c);
}

void BillboardShader::loadSize(const glm::vec2 s) {
    this->loadVec2(getUniform("size"), s);
}

void BillboardShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}