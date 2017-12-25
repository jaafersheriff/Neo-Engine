#include "AtmosphereRenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool AtmosphereRenderer::activate(Atmosphere *atm) {
    this->atmosphere = atm;
    shader = new AtmosphereShader;
    return shader->init();
}

void AtmosphereRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    AtmosphereShader *aShader = dynamic_cast<AtmosphereShader*>(shader);
    aShader->loadP(projection);

    /* Update view matrix so geodisc is always centered at camera */
    glm::mat4 atmV = glm::mat4(*view);
    atmV[3][0] = atmV[3][1] = atmV[3][2] = 0;
    aShader->loadV(&atmV);
}

void AtmosphereRenderer::prepare() {
    /* Nothing to prepare */
}

void AtmosphereRenderer::render(const World *world) {
    AtmosphereShader *aShader = dynamic_cast<AtmosphereShader*>(shader);

    /* World light */
    aShader->loadLight(world->light);

    /* Model matrix */
    glm::mat4 M = glm::scale(glm::mat4(1.f), glm::vec3(1000.f));
    aShader->loadM(&M);

    /* Bind vertices */
    glBindVertexArray(atmosphere->mesh->vaoId);
    int pos = shader->getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, atmosphere->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Bind indices */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atmosphere->mesh->eleBufId);

    /* Bind textures */
    aShader->loadGlowTexture(atmosphere->glowTexture);
    glActiveTexture(GL_TEXTURE0 + atmosphere->glowTexture->textureId);
    glBindTexture(GL_TEXTURE_2D, atmosphere->glowTexture->textureId);
    aShader->loadColorTexture(atmosphere->colorTexture);
    glActiveTexture(GL_TEXTURE0 + atmosphere->colorTexture->textureId);
    glBindTexture(GL_TEXTURE_2D, atmosphere->colorTexture->textureId);

    /* Draw */
    glDrawElements(GL_TRIANGLES, (int) atmosphere->mesh->eleBuf.size(), GL_UNSIGNED_INT, (const void *) 0);
}

void AtmosphereRenderer::cleanUp() {
    shader->cleanUp();
}
