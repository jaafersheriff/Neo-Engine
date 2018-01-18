#include "BoundingSphereShader.hpp"
#include "Toolbox/MeshGenerator.hpp"

bool BoundingSphereShader::init(std::vector<BoundingSphere *> *spheres) {
    /* Parent init */
    if (!spheres || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->spheres = spheres;

    addAllLocations();

    /* Create sphere mesh */
    this->sphere = MeshGenerator::generateSphere(2);

    // TODO
    return true;
}

void BoundingSphereShader::addAllLocations() {
    addAttribute("vertexPos");

    addUniform("P");
    addUniform("M");
    addUniform("V");
}

void BoundingSphereShader::setGlobals(const glm::mat4 *P, const glm::mat4 *V) {
    loadP(P);
    loadV(V);
}

void BoundingSphereShader::render(const World *world) {

    glm::mat4 M;

    for (auto &s : *spheres) {

    }
}