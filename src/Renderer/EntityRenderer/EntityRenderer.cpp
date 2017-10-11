#include "EntityRenderer.hpp"
#include "World/EntityWorld.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "glm/gtx/string_cast.hpp"
#include <iostream>

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

void EntityRenderer::render(World *world) {
   EntityShader *eShader = dynamic_cast<EntityShader*>(shader);
   EntityWorld *eWorld = dynamic_cast<EntityWorld*>(world);

   // There's only one light as of now
   eShader->loadLight(eWorld->light);

   glm::mat4 M;
   // TODO : batched render
   for (unsigned int i = 0; i < entitiesPointer->size(); i++) {
      Entity *e = &(*entitiesPointer)[i];

      if (!e->mesh.size()) {
         continue;
      }

      // Model matrix
      M = glm::mat4(1.f);
      M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.x), glm::vec3(1, 0, 0));
      M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.y), glm::vec3(0, 1, 0));
      M *= glm::rotate(glm::mat4(1.f), glm::radians(e->rotation.z), glm::vec3(0, 0, 1));
      M *= glm::scale(glm::mat4(1.f), e->scale);
      M *= glm::translate(glm::mat4(1.f), e->position);

      // Bind texture/material to shader
      prepareTexture(e->texture);
      eShader->loadM(&M);

      // Loop through all shapes in mesh
      for (int i = 0; i < e->mesh.size(); i++) {
         // Bind shape
         prepareMesh(e->mesh[i]);
         // Draw shape
         glDrawElements(GL_TRIANGLES, (int)e->mesh[i]->eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
         // Unbind shape
         unPrepareMesh(e->mesh[i]);
      }

      // Unbind texture
      unPrepareTexture(e->texture);
   }
}

// All Meshes are assumed to have valid vertices and element indices
// For Entities we assume meshes to include normal data
void EntityRenderer::prepareMesh(Mesh *mesh) {

   glBindVertexArray(mesh->vaoId);
   
   // Bind position buffer
   int pos = shader->getAttribute("vertexPos");
   glEnableVertexAttribArray(pos);
   glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId);
   glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

   // Bind normal buffer
   pos = shader->getAttribute("vertexNormal");
   if (pos != -1 && mesh->norBufId != 0) {
      glEnableVertexAttribArray(pos);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->norBufId);
      glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
   }

   // Bind texture buffer
   pos = shader->getAttribute("vertexTexture");
   if (pos != -1 && mesh->texBufId != 0) {
      glEnableVertexAttribArray(pos);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->texBufId);
      glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
   }

   // Bind element buffer
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId);
}

void EntityRenderer::prepareTexture(ModelTexture &texture) {
   EntityShader *eShader = dynamic_cast<EntityShader*>(shader);

   if(texture.textureImage.textureId != 0) {
      eShader->loadUsesTexture(true);
      eShader->loadTexture(texture.textureImage);
   }
   else {
      eShader->loadUsesTexture(false);
   }

   // Material
   eShader->loadMaterial(texture.diffuseColor, 
                         texture.specularColor);
   eShader->loadShine(texture.shineDamper);
}

void EntityRenderer::unPrepareMesh(Mesh *mesh) {
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

void EntityRenderer::unPrepareTexture(ModelTexture &texture) {
   glActiveTexture(GL_TEXTURE0 + texture.textureImage.textureId);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void EntityRenderer::cleanUp() {
   shader->cleanUp();
}