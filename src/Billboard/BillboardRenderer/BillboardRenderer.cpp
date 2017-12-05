#include "BillboardRenderer.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
    sort();
}

void BillboardRenderer::sort() {
    for (int i = 0; i < billboards->size(); i++) {
        int minSize = i; 
        for (int j = i; j < billboards->size(); j++) {
            if (billboards->at(j)->distance > billboards->at(i)->distance) {
                minSize = j;
            }
        }
        if (minSize != i) {
            Billboard *tmp = billboards->at(i);
            billboards->at(i) = billboards->at(minSize);
            billboards->at(minSize) = tmp;
        }
    }
}

void BillboardRenderer::render(const World *world) {
    if(!billboards->size()) {
        return;
    }

    BillboardShader *bShader = dynamic_cast<BillboardShader *>(shader);
    glDisable(GL_DEPTH_TEST);

    /* Iterate through billboards */
    glm::mat4 M;
    for (auto billboard : *billboards) {
        /* Bind vertices & texture coords */
        // TODO : why do i need to do this per billboard?
        glBindVertexArray(billboard->mesh->vaoId);
        int pos = shader->getAttribute("vertexPos");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, billboard->mesh->vertBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

       /* Load billboard's position and size */
        bShader->loadCenter(billboard->center);
        bShader->loadSize(billboard->size);

        /* Load billboard's rotation */
        M = glm::mat4(1.f);
        M *= glm::rotate(glm::mat4(1.f), glm::radians(billboard->rotation), glm::vec3(0, 0, 1));
        bShader->loadM(&M);

        /* Load texture */
        bShader->loadTexture(billboard->texture);
        glActiveTexture(GL_TEXTURE0 + billboard->texture->textureId);
        glBindTexture(GL_TEXTURE_2D, billboard->texture->textureId);

        /* Draw */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, billboard->mesh->vertBuf.size() / 3);

        /* Unbind */
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }
    glEnable(GL_DEPTH_TEST);
}

void BillboardRenderer::cleanUp() {
    shader->cleanUp();
}