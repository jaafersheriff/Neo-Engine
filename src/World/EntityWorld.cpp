#include "EntityWorld.hpp"

void EntityWorld::init(Loader &loader) {
   // Create mesh
   Mesh *bunnyMesh = loader.loadObjMesh("../resources/Model.obj");
   Mesh *cubeMesh  = loader.loadObjMesh("../resources/cube.obj");
   // TODO: create texture

   // Add entity to scene
   for (int i = 1; i < 2; i++) {
      for (int j = 2; j < 3; j++) {
         for (int k = 5; k < 6; k++) {
            Entity e(bunnyMesh, glm::vec3(i*3, j*3, k*3), glm::vec3(180, 0, 0), glm::vec3(10, 10, 10));
            // TODO : attach texture to entity
            e.texture.ambientColor = e.texture.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
            e.texture.specularColor = glm::vec3(1.f, 1.f, 1.f);
            e.texture.shineDamper = 1.f;
            entities.push_back(e);
         }
      }
   }

   // Add light
   glm::vec3 lightPos(6, 20, 45);
   Light light(lightPos, glm::vec3(1.f, 1.f, 1.f));
   light.attenuation = glm::vec3(1.f, 0.0f, 0.0f);
   Entity lightModel(cubeMesh, lightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.5f, 0.5f, 0.5f));
   lightModel.texture.ambientColor = lightModel.texture.diffuseColor = lightModel.texture.specularColor = glm::vec3(1.f, 1.f, 1.f);
   lights.push_back(light);
   entities.push_back(lightModel);
}

void EntityWorld::prepareRenderer(MasterRenderer &mr) {
   mr.activateEntityRenderer(&entities);
}

void EntityWorld::update(Context &ctx) {
   takeInput(ctx.mouse, ctx.keyboard);
   if (isPaused) {
      return;
   }

   camera.update();

   for (unsigned int i = 0; i < entities.size(); i++) {
      entities[i].update();
   }
}

void EntityWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
   if (keyboard.isKeyPressed(' ')) {
      isPaused = !isPaused;
   }
   if (isPaused) {
      return;
   }
   if (mouse.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
      camera.updateLookAt(mouse.dx, mouse.dy);
   }
   if (keyboard.isKeyPressed('w')) {
      camera.moveForward();
   }
   if (keyboard.isKeyPressed('a')) {
      camera.moveLeft();
   }
   if (keyboard.isKeyPressed('s')) {
      camera.moveBackward();
   }
   if (keyboard.isKeyPressed('d')) {
      camera.moveRight();
   }
   if (keyboard.isKeyPressed('e')) {
      camera.moveDown();
   }
   if (keyboard.isKeyPressed('r')) {
      camera.moveUp();
   }
}

void EntityWorld::cleanUp() {
   
}