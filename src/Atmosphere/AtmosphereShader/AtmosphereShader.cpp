#include "AtmosphereShader.hpp"

bool AtmosphereShader::init() {
    if (!Shader::init()) {
        return false;
    }

    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");
    addUniform("M");

    /* Light */
    addUniform("lightPos");

    /* Textures */
    addUniform("glowTexture");
    addUniform("colorTexture");

    return true;
}

void AtmosphereShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void AtmosphereShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void AtmosphereShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void AtmosphereShader::loadLight(const Light *l) {
    this->loadVec3(getUniform("lightPos"), l->position);
}

void AtmosphereShader::loadColorTexture(const Texture *t) {
    this->loadInt(getUniform("colorTexture"), t->textureId);
}

void AtmosphereShader::loadGlowTexture(const Texture *t) {
    this->loadInt(getUniform("glowTexture"), t->textureId);
}
