#include "MasterRenderer.hpp"

#include "World/World.hpp"
#include "Renderer/Renderer.hpp"

#include "Entity/EntityRenderer/EntityRenderer.hpp"

#include "Shader/GLSL.hpp"

#include "glm/gtc/matrix_transform.hpp"

MasterRenderer::MasterRenderer() {
}

void MasterRenderer::render(const Display &display, World *world) {
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   // Reset rendering display
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glClearColor(0.f, 0.f, 0.f, 1.f);

   // Create view matrix
   const glm::mat4 v = glm::lookAt(world->camera.position, world->camera.lookAt, glm::vec3(0, 1, 0));

   for (auto renderer = renderers.begin(); renderer != renderers.end(); renderer++) {
      (*renderer)->shader->bind();
      (*renderer)->prepare();
      (*renderer)->setGlobals(&display.projectionMatrix, &v);
      (*renderer)->render(world);
      (*renderer)->shader->unbind();
   }
}

void MasterRenderer::activateEntityRenderer(std::vector<Entity> *entities) {
   EntityRenderer *eR = new EntityRenderer;
   eR->activate(entities);
   renderers.push_back(eR);
}

void MasterRenderer::cleanUp() {
   for (auto renderer = renderers.begin(); renderer != renderers.end(); renderer++) {
      (*renderer)->cleanUp();
   }
}