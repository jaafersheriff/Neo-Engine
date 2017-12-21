#include "SkyboxRenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool SkyboxRenderer::activate(Skybox *skybox) {
    this->skybox = skybox;
    shader = new SkyboxShader;
    return shader->init();
}

void SkyboxRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    SkyboxShader *sShader = dynamic_cast<SkyboxShader*>(shader);
    sShader->loadP(projection);

    /* Create skybox view matrix */
    glm::mat4 skyboxV = glm::mat4(*view);
    skyboxV[3][0] = skyboxV[3][1] = skyboxV[3][2] = 0;
    skyboxV *= glm::rotate(glm::mat4(1.f), glm::radians(skybox->rotation), glm::vec3(0, 1, 0));
    sShader->loadV(&skyboxV);
}

void SkyboxRenderer::prepare() {
    /* Nothing to prepare */
}

void SkyboxRenderer::render(const World *world) {
    SkyboxShader *sShader = dynamic_cast<SkyboxShader*>(shader);

    /* Bind vertices */
    glBindVertexArray(skybox->mesh->vaoId);
    int pos = shader->getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

    /* Bind texture */
    sShader->loadCubeTexture(skybox->cubeTexture);
    glActiveTexture(GL_TEXTURE0 + skybox->cubeTexture->textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubeTexture->textureId);
    
    /* Draw */
    glDrawArrays(GL_TRIANGLES, 0, skybox->mesh->vertBuf.size() / 3);

    /* Unbind */
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

void SkyboxRenderer::cleanUp() {
    shader->cleanUp();
}
