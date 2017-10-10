#include "EntityWorld.hpp"

void EntityWorld::init(Loader &loader) {
   // Create entity
   Entity e(loader.loadObjMesh("../resources/sphere.obj"), glm::vec3(30, -5, -5), glm::vec3(0, 90, 0), glm::vec3(10, 10, 10));
   e.texture.textureImage = loader.loadPngTexture("../resources/world.bmp");
   entities.push_back(e);
   
   e = Entity(loader.loadObjMesh("../resources/Model.obj"), glm::vec3(30, -5, 10), glm::vec3(180, 90, 0), glm::vec3(10, 10, 10));
   e.texture.textureImage = loader.loadPngTexture("../resources/Model.png");
   entities.push_back(e);

   e = Entity(loader.loadObjMesh("../resources/mr_krab.obj"), glm::vec3(30, 8, 5), glm::vec3(0, 270, 0), glm::vec3(10, 10, 10));
   e.texture.textureImage = loader.loadPngTexture("../resources/mr_krab.png");
   entities.push_back(e);

   // Set up light
   light.position = glm::vec3(10, 10, 0);
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