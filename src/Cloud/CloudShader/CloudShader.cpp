#include "CloudShader.hpp"

bool CloudShader::init() {
    if (!Shader::init()) {
        return false;
    }

    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");
    addUniform("M");

    /* Billboard things */
    addUniform("center");
    addUniform("size");

    /* Texture */
    addUniform("textureImage");

    /* Light */
    addUniform("lightPos");
    addUniform("lightCol");

    return true;
}

void CloudShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void CloudShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void CloudShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void CloudShader::loadCenter(const glm::vec3 c) {
    this->loadVec3(getUniform("center"), c);
}

void CloudShader::loadSize(const glm::vec2 s) {
    this->loadVec2(getUniform("size"), s);
}

void CloudShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}

void CloudShader::loadLight(const Light *light) {
    this->loadVec3(getUniform("lightPos"), light->position);
    this->loadVec3(getUniform("lightCol"), light->color);
}
