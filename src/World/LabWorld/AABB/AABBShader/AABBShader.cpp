#include "AABBShader.hpp"

bool AABBShader::init(std::vector<Block *> *blocks) {
    /* Parent init */
    if (!blocks || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->blocks = blocks;

    addAllLocations();

    return true;
}

void AABBShader::addAllLocations() {
    // TODO
}

void AABBShader::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    loadP(projection);
    loadV(view);
}

void AABBShader::render(const World *world) {
    for (auto &b : *blocks) {
        AABB boundingBox = b->boundingBox;
        // TODO
    }
}

void AABBShader::cleanUp() {
    Shader::cleanUp();
}

void AABBShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void AABBShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}