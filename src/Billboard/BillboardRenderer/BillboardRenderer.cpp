#include "BillboardRenderer.hpp"

void BillboardRenderer::activate(std::vector<Billboard *> *billboards) {
    this->billboards = billboards;
    shader = new BillboardShader;
    shader->init();
}

void BillboardRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    BillboardShader *bShader = dynamic_cast<BillboardShader *>(shader);

    bShader->loadP(projection);
    bShader->loadV(view);
}

void BillboardRenderer::prepare() {

}

void BillboardRenderer::render(const World *world) {
    if(!billboards->size()) {
        return;
    }

    BillboardShader *bShader = dynamic_cast<BillboardShader *>(shader);

    /* Iterate through billboards */
    for (auto billboard : *billboards) {
        /* Bind vertices */
        // TODO : why do i need to do this per billboard?
        glBindVertexArray(billboard->mesh->vaoId);
        int pos = shader->getAttribute("vertexPos");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, billboard->mesh->vertBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

       /* Load billboard's position and size */
        bShader->loadCenter(billboard->center);
        bShader->loadSize(billboard->size);

        /* Load texture */
        bShader->loadTexture(billboard->texture);
        glActiveTexture(GL_TEXTURE0 + billboard->texture.textureId);
        glBindTexture(GL_TEXTURE_2D, billboard->texture.textureId);

        /* Draw */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, billboard->mesh->vertBuf.size() / 3);

        /* Unbind */
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }
}

void BillboardRenderer::cleanUp() {
    shader->cleanUp();
}