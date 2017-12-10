#include "SunShader.hpp"

bool SunShader::init() {
    if (!Shader::init()) {
        return false;
    }

    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");

    /* Billboard things */
    addUniform("center");
    addUniform("size");

    /* Texture */
    addUniform("textureImage");

    return true;
}

void SunShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void SunShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void SunShader::loadCenter(const glm::vec3 c) {
    this->loadVec3(getUniform("center"), c);
}

void SunShader::loadSize(const glm::vec2 s) {
    this->loadVec2(getUniform("size"), s);
}

void SunShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}