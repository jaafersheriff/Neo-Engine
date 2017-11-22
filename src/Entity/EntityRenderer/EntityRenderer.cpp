#include "EntityRenderer.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

void EntityRenderer::activate(std::vector<Entity> *ep) {
    this->entitiesPointer = ep;
    shader = new EntityShader;
    shader->init();
}

void EntityRenderer::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    EntityShader *eShader = dynamic_cast<EntityShader*>(shader);
    eShader->loadP(projection);
    eShader->loadV(view);
}

void EntityRenderer::prepare() {
    // TODO : batched render
}

void EntityRenderer::render(const World *world) {
    EntityShader *eShader = dynamic_cast<EntityShader*>(shader);

    /* There's only one light in world */
    eShader->loadLight(world->light);

    /* Loop through every entity */
    // TODO : batched render
    glm::mat4 M;
    for(auto e : *entitiesPointer) {
        /* If entity mesh doesn't contain geometry, skip it */
        if (!e.mesh || !e.mesh->vertBuf.size()) {
            continue;
        }

        /* Create model matrix for this entity */
        M = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), e.position);
        M *= glm::rotate(glm::mat4(1.f), glm::radians(e.rotation.x), glm::vec3(1, 0, 0));
        M *= glm::rotate(glm::mat4(1.f), glm::radians(e.rotation.y), glm::vec3(0, 1, 0));
        M *= glm::rotate(glm::mat4(1.f), glm::radians(e.rotation.z), glm::vec3(0, 0, 1));
        M *= glm::scale(glm::mat4(1.f), e.scale);
        eShader->loadM(&M);

        /* Prepare texture/material */
        prepareTexture(e.texture);

        /* Prepare mesh */
        prepareMesh(e.mesh);

        /* render */
        glDrawElements(GL_TRIANGLES, (int)e.mesh->eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);

        /* Clean up mesh */
        unPrepareMesh(e.mesh);

        /* Clean up texture */
        unPrepareTexture(e.texture);
    }
}

/* All Meshes are assumed to have valid vertices and element indices
 * For Entities we assume meshes to include normal data */
void EntityRenderer::prepareMesh(const Mesh *mesh) {
    /* Bind mesh VAO */
    glBindVertexArray(mesh->vaoId);
    
    /* Bind vertex buffer VBO */
    int pos = shader->getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Bind normal buffer VBO */
    pos = shader->getAttribute("vertexNormal");
    if (pos != -1 && mesh->norBufId != 0) {
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->norBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    /* Bind texture coordinate buffer VBO */
    pos = shader->getAttribute("vertexTexture");
    if (pos != -1 && mesh->texBufId != 0) {
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->texBufId);
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    /* Bind indices buffer VBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId);
}

void EntityRenderer::prepareTexture(const ModelTexture &texture) {
    EntityShader *eShader = dynamic_cast<EntityShader*>(shader);

    /* Bind texture if it exists */
    if(texture.textureImage.textureId != 0) {
        eShader->loadUsesTexture(true);
        eShader->loadTexture(texture.textureImage);
    }
    else {
        eShader->loadUsesTexture(false);
    }

    /* Bind materials */
    eShader->loadMaterial(texture.ambientColor, 
                          texture.diffuseColor, 
                          texture.specularColor);
    eShader->loadShine(texture.shineDamper);
}

void EntityRenderer::unPrepareMesh(const Mesh *mesh) {
    glDisableVertexAttribArray(shader->getAttribute("vertexPos"));

    int pos = shader->getAttribute("vertexNormal");
    if (pos != -1) {
        glDisableVertexAttribArray(pos);
    }

    pos = shader->getAttribute("vertexTexture");
    if (pos != -1) {
        glDisableVertexAttribArray(pos);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EntityRenderer::unPrepareTexture(const ModelTexture &texture) {
    glActiveTexture(GL_TEXTURE0 + texture.textureImage.textureId);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void EntityRenderer::cleanUp() {
    shader->cleanUp();
}