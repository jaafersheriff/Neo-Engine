#pragma once
#ifndef _ENTITY_WORLD_HPP_
#define _ENTITY_WORLD_HPP_

#include "World/World.hpp"
#include "Entity/Entity.hpp"
#include "Toolbox/Toolbox.hpp"

#include <iostream>

class EntityWorld : public World {
   public:
      std::vector<Entity> entities; // TODO - map<mesh, entity>

      EntityWorld() : World("Entity World") { }

      void init(Loader &loader) {
         // Create mesh
         Mesh *bunnyMesh = loader.loadObjMesh("../resources/penguinhat.obj");

         // Add entity to scene
         Entity e(bunnyMesh, glm::vec3(5, 0, 0), glm::vec3(0, -90, 0), glm::vec3(1, 1, 1));
         e.texture.ambientColor = e.texture.diffuseColor = glm::vec3(0.41f, 0.12f, 0.77f);
         e.texture.specularColor = glm::vec3(0.f, 0.f, 1.f);
         e.texture.shineDamper = 2.f;
         entities.push_back(e);

         // Add light
         lights.push_back(Light(glm::vec3(1000, 1000, 0)));
      }

      void prepareRenderer(MasterRenderer &mr) {
         mr.activateEntityRenderer(&entities);
      }  
      
      void update(Context &ctx) {
         camera.update();
         takeInput(ctx.mouse, ctx.keyboard);
         
         for (unsigned int i = 0; i < entities.size(); i++) {
            entities[i].update();
         }

      }

      void takeInput(Mouse &mouse, Keyboard &keyboard) {
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

      void cleanUp() {

      }

};

#endif