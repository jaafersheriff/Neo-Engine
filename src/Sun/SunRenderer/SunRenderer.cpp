#include "SunRenderer.hpp"

bool SunRenderer::activate(Sun *sun) {
    this->sun = sun;
    shader = new SunShader;
    return shader->init();
}

void SunRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    SunShader *sShader = dynamic_cast<SunShader *>(shader);

    sShader->loadP(projection);
    sShader->loadV(view);
}

void SunRenderer::prepare() {
    /* Nothing to prepare */
}

void SunRenderer::render(const World *world) {
    SunShader *sShader = dynamic_cast<SunShader *>(shader);

    glDisable(GL_DEPTH_TEST);

    /* Bind vertices */
    glBindVertexArray(sun->mesh->vaoId);
    int pos = shader->getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, sun->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Load billboard's position and size */
    sShader->loadCenter(sun->center);
    sShader->loadSize(sun->size);

    /* Load texture */
    sShader->loadTexture(sun->texture);
    glActiveTexture(GL_TEXTURE0 + sun->texture->textureId);
    glBindTexture(GL_TEXTURE_2D, sun->texture->textureId);

    /* Draw */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sun->mesh->vertBuf.size() / 3);

    /* Unbind */
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void SunRenderer::cleanUp() {
    shader->cleanUp();
}