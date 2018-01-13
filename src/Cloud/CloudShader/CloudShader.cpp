#include "CloudShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool CloudShader::init(std::vector<CloudBillboard *> *billboards) {
    /* Parent init */
    if (!billboards || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->billboards = billboards;

    /* Set enum type */
    this->type = MasterRenderer::ShaderTypes::CLOUD_SHADER;

    addAllLocations();

    return true;
}

void CloudShader::addAllLocations() {
    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");
    addUniform("M");

    /* Billboard things */
    addUniform("center");
    addUniform("size");

    /* Texture */
    addUniform("textureImage");

    /* Light */
    addUniform("lightPos");
    addUniform("lightCol");
}

/* Painters algorithm */
void CloudShader::sortByDistance() {
     for (unsigned int i = 0; i < billboards->size(); i++) {
         int minSize = i; 
         for (unsigned int j = i; j < billboards->size(); j++) {
             if (billboards->at(j)->distance > billboards->at(minSize)->distance) {
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

void CloudShader::render() {
    /* Skip rendering if no billboards exist */
    if (!billboards->size()) {
        return;
    }

    /* Handle stacked transparency */
    glDisable(GL_DEPTH_TEST);

    /* Iterate through every cloud billboards */
    glm::mat4 M;
    for (auto billboard : *billboards) {
        /* Bind vertices */
        // TODO : why do i need to do this per billboard?
        glBindVertexArray(billboard->mesh->vaoId);
        int pos = getAttribute("vertexPos");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, billboard->mesh->vertBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

        /* Load billboard's position and size */
        loadCenter(billboard->center);
        loadSize(billboard->size);

        /* Load billboard's rotation */
        M = glm::rotate(glm::mat4(1.f), glm::radians(billboard->rotation), glm::vec3(0, 0, 1));
        loadM(&M);

        /* Load texture */
        loadTexture(billboard->texture);
        glActiveTexture(GL_TEXTURE0 + billboard->texture->textureId);
        glBindTexture(GL_TEXTURE_2D, billboard->texture->textureId);

        /* Draw */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, billboard->mesh->vertBuf.size() / 3);

        /* Unbind */
        glDisableVertexAttribArray(getAttribute("vertexPos"));
        Shader::unloadMesh();
        Shader::unloadTexture(billboard->texture->textureId);
    }
}

void CloudShader::cleanUp() {
    Shader::cleanUp();
}

void CloudShader::loadM(const glm::mat4 *m) {
    this->loadMat4(getUniform("M"), m);
}

void CloudShader::loadCenter(const glm::vec3 c) {
    this->loadVec3(getUniform("center"), c);
}

void CloudShader::loadSize(const glm::vec2 s) {
    this->loadVec2(getUniform("size"), s);
}

void CloudShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}
