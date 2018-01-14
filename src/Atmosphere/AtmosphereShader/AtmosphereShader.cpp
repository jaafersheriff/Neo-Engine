#include "AtmosphereShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool AtmosphereShader::init(Atmosphere *atm) {
    /* Parent init */
    if (!atm || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->atmosphere = atm;

    /* Set type */
    this->type = MasterRenderer::ShaderTypes::ATMOSPHERE_SHADER;

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

void AtmosphereShader::render(const World *world) {
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
void AtmosphereShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void AtmosphereShader::loadColorTexture(const Texture *t) {
    this->loadInt(getUniform("colorTexture"), t->textureId);
}

void AtmosphereShader::loadGlowTexture(const Texture *t) {
    this->loadInt(getUniform("glowTexture"), t->textureId);
}
