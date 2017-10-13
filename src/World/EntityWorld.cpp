#include "EntityWorld.hpp"

void EntityWorld::init(Loader &loader) {
   Entity e = Entity(loader.loadObjMesh("../resources/cube.obj"), glm::vec3(0), glm::vec3(0), glm::vec3(50.f, 1, 50.f));
   e.texture.diffuseColor = glm::vec3(1.f);
   entities.push_back(e);

   // Set up light
   light.position = glm::vec3(0, 10, 0);
   e = Entity(loader.loadObjMesh("../resources/sphere.obj"), light.position, glm::vec3(0), glm::vec3(1, 1, 1));
   e.texture.diffuseColor = e.texture.specularColor = glm::vec3(1.f);
   e.texture.shineDamper = 0.2f;
   //entities.push_back(e);
   light.color = glm::vec3(1.f);
   light.attenuation = glm::vec3(1.f, 0.0f, 0.0f);
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