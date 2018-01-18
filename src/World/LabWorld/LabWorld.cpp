#include "LabWorld.hpp"

#include "Block.hpp"
#include "Player.hpp"

#include <iostream>

void LabWorld::init(Loader &loader) {
    this->loader = &loader;

    /* Camera */
    this->camera = new Camera();

    /* Main light source */
    this->light = new Light(glm::vec3(-1000, 1000, 1000), glm::vec3(1.f));

    /* Terrain */
    ModelTexture mt(0.3f,
                    glm::vec3(1.f, 0.f, 0.f), 
                    glm::vec3(0.f, 0.f, 1.f));
    entities.push_back(new Entity(loader.loadObjMesh("cube.obj"),
                                  mt, 
                                  glm::vec3(0.f, -1.f, 0.f),
                                  glm::vec3(0.f),
                                  glm::vec3(100.f, 0.f, 100.f)));
    entities[0]->update();
    /* Player */
    player = new Player(camera, BoundingSphere(loader.loadObjMesh("cube.obj")));
}

void LabWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateEntityShader(&entities);
}

void LabWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    camera->update();

    /* Add game objects at a certain time step */
    // TODO : timestep
    if (gameObjects < MAX_GAME_OBJECTS) {
        /* Randomize position and rotation */
        glm::vec3 pos = glm::vec3(Toolbox::genRandom(-50, 50.f), 1.f, Toolbox::genRandom(-50.f, 50.f));
        float rotation = Toolbox::genRandom(0.f, 360.f);

        /* Create game object */
        Block *b = new Block(loader->loadObjMesh("bunny.obj"),          /* Mesh */
                                     alive,                             /* Texture */
                                     pos,                               /* Position */
                                     rotation,                          /* Rotation */
                                     glm::vec3(2.f),                    /* Scale */
                                     Toolbox::genRandom(0.2f, 0.9f));   /* Velocity */

        entities.push_back(b);
        blocks.push_back(b);
        spheres.push_back(&b->boundingSphere);
        gameObjects++;
    }

    player->update();
    for (auto block : blocks) {
        block->update(entities[0], player->boundingSphere);
    }
}

void LabWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
    if (mouse.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        camera->takeMouseInput(mouse.dx, mouse.dy);
    }
    else {
        camera->takeMouseInput(0.f, 0.f);
    }
    if (keyboard.isKeyPressed('w')) {
        camera->moveForward();
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('a')) {
        camera->moveLeft();
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('s')) {
        camera->moveBackward();
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('d')) {
        camera->moveRight();
        camera->position.y = 0.f;
    }
}

void LabWorld::cleanUp() {

}