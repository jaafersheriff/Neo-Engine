#include "AtmosphereShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool AtmosphereShader::init(Atmosphere *atm) {
    /* Parent init */
    if (!Shader::init()) {
        return false;
    }

    /* Set render target */
    this->atmosphere = atm;

    addAllLocations();

    return true;
}

void AtmosphereShader::addAllLocations() {
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
}

void AtmosphereShader::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    loadP(projection);

    /* Update view matrix so geodisc is always centered at camera */
    glm::mat4 newView = glm::mat4(*view);
    newView[3][0] = newView[3][1] = newView[3][2] = 0.f;
    loadV(&newView);
}

void AtmosphereShader::render(const World *world) {
    /* World light */
    loadLight(world->light);

    /* Model matrix */
    glm::mat4 M = glm::scale(glm::mat4(1.f), glm::vec3(atmosphere->size));
    loadM(&M);

    /* Bind vertices */
    glBindVertexArray(atmosphere->mesh->vaoId);
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, atmosphere->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Bind indices */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atmosphere->mesh->eleBufId);

    /* Bind textures */
    loadGlowTexture(atmosphere->glowTexture);
    glActiveTexture(GL_TEXTURE0 + atmosphere->glowTexture->textureId);
    glBindTexture(GL_TEXTURE_2D, atmosphere->glowTexture->textureId);
    loadColorTexture(atmosphere->colorTexture);
    glActiveTexture(GL_TEXTURE0 + atmosphere->colorTexture->textureId);
    glBindTexture(GL_TEXTURE_2D, atmosphere->colorTexture->textureId);

    /* Draw */
    glDrawElements(GL_TRIANGLES, (int) atmosphere->mesh->eleBuf.size(), GL_UNSIGNED_INT, (const void *) 0);

    /* Unbind mesh */
    glDisableVertexAttribArray(getAttribute("vertexPos"));
    Shader::unloadMesh();

    /* Unbind textures */
    Shader::unloadTexture(atmosphere->glowTexture->textureId);
    Shader::unloadTexture(atmosphere->colorTexture->textureId);
}

void AtmosphereShader::cleanUp() {
    Shader::cleanUp();
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
