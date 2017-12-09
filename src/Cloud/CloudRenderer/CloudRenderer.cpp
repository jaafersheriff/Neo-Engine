#include "CloudRenderer.hpp"
#include "glm/gtc/matrix_transform.hpp"

bool CloudRenderer::activate(std::vector<CloudBillboard *> *billboards) {
    this->billboards = billboards;
    shader = new CloudShader;
    return shader->init();
}

void CloudRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    CloudShader *cShader = dynamic_cast<CloudShader *>(shader);

    cShader->loadP(projection);
    cShader->loadV(view);
}

void CloudRenderer::prepare() {
    /* Sort */
    for (int i = 0; i < billboards->size(); i++) {
        int minSize = i; 
        for (int j = i; j < billboards->size(); j++) {
            if (billboards->at(j)->distance > billboards->at(i)->distance) {
                minSize = j;
            }
        }
        if (minSize != i) {
            CloudBillboard *tmp = billboards->at(i);
            billboards->at(i) = billboards->at(minSize);
            billboards->at(minSize) = tmp;
        }
    }
}

void CloudRenderer::render(const World *world) {
    if(!billboards->size()) {
        return;
    }

    CloudShader *cShader = dynamic_cast<CloudShader *>(shader);
    cShader->loadCameraPosition(world->camera.position);

    glDisable(GL_DEPTH_TEST);
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
        cShader->loadCenter(billboard->center);
        cShader->loadSize(billboard->size);

        /* Load billboard's rotation */
        M = glm::rotate(glm::mat4(1.f), glm::radians(billboard->rotation), glm::vec3(0, 0, 1));
        cShader->loadM(&M);

        /* Load texture */
        cShader->loadTexture(billboard->texture);
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

void CloudRenderer::cleanUp() {
    shader->cleanUp();
}