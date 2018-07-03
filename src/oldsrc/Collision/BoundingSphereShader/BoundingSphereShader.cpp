#include "BoundingSphereShader.hpp"
#include "Toolbox/MeshGenerator.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool BoundingSphereShader::init(std::vector<BoundingSphere *> *spheres) {
    /* Parent init */
    if (!spheres || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->spheres = spheres;

    addAllLocations();

    /* Create sphere mesh */
    this->sphereMesh = MeshGenerator::generateSphere(0);

    return true;
}

void BoundingSphereShader::addAllLocations() {
    addAttribute("vertexPos");

    addUniform("P");
    addUniform("M");
    addUniform("V");
}

void BoundingSphereShader::render() {
    /* Bind mesh VAO */
    glBindVertexArray(sphereMesh->vaoId);
    
    /* Bind vertex buffer VBO */
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, sphereMesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Bind indices */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereMesh->eleBufId);

    glm::mat4 M;
    for (auto &s : *spheres) {
        /* Set M */
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), s->position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(s->radius));
        loadM(&M);

        /* Draw sphere */
        glDrawElements(GL_TRIANGLES, (int)sphereMesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr);
    }

    glDisableVertexAttribArray(getAttribute("vertexPos"));
    Shader::unloadMesh();
}

void BoundingSphereShader::cleanUp() {
    Shader::cleanUp();
}

void BoundingSphereShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}
