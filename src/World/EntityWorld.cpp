#include "EntityWorld.hpp"

void EntityWorld::init(Loader &loader) {
   // Load obj meshes
   Mesh *bunnyMesh = loader.loadObjMesh("../resources/Model.obj");
   Mesh *cubeMesh  = loader.loadObjMesh("../resources/cube.obj");

   // Create texture
   ModelTexture texture;
   texture.ambientColor = texture.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
   texture.specularColor = glm::vec3(1.f, 1.f, 1.f);
   texture.shineDamper = 1.f;
   // Load png texture
   // texture.textureImage = loader.loadPngTexture("../resources/Model.png");

   // entities.push_back(Entity(bunnyMesh, texture, glm::vec3(6, 20, 30), glm::vec3(180, 0, 0), glm::vec3(10, 10, 10)));

   // entities.push_back(Entity(bunnyMesh, glm::vec3(60, 20, 60), glm::vec3(180, 0, 0), glm::vec3(10, 10, 10)));

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