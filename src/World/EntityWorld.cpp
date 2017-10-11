#include "EntityWorld.hpp"

void EntityWorld::init(Loader &loader) {
   Entity e = Entity(loader.loadObjMesh("../resources/millenium-falcon.obj"), glm::vec3(-10, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0.05f, 0.05f, 0.05f));
   e.texture.textureImage = loader.loadTexture("../resources/falcon.jpg");
   e.texture.specularColor = glm::vec3(0.3f);
   e.texture.shineDamper = 0.8f;
   entities.push_back(e);

   // Set up light
   light.position = glm::vec3(-10, 100, 0);
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
      entities[i].rotation.z += 1.f;
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