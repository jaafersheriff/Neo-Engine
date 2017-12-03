#include "SkyboxShader.hpp"

bool SkyboxShader::init() {
    if (!Shader::init()) {
        return false;
    }

    /* Attributes */
    addAttribute("vertexPos");

    /* Matrix uniforms */
    addUniform("P");
    addUniform("V");

    /* Cube texture */
    addUniform("cubeMap");

    return true;
}

void SkyboxShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void SkyboxShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void SkyboxShader::loadCubeTexture(const CubeTexture &ct) {
    this->loadInt(getUniform("cubeMap"), ct.textureId);
}
