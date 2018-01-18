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
                    glm::vec3(0.f, 0.6f, 0.2f), 
                    glm::vec3(0.f, 0.f, 1.f));
    entities.push_back(new Entity(loader.loadObjMesh("cube.obj"),
                                  mt, 
                                  glm::vec3(0.f, -1.f, 0.f),
                                  glm::vec3(0.f),
                                  glm::vec3(100.f, 0.f, 100.f)));
    entities[0]->update();

    /* Player */
    player = new Player(camera, BoundingSphere(loader.loadObjMesh("cube.obj")));

    /* Store game object mesh */
    this->gameObjectMesh = loader.loadObjMesh("bunny.obj");
}

void LabWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateEntityShader(&entities);
    mr->activateBoundingSphereShader(&spheres);
}

void LabWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard, ctx.timeStep);
    camera->update();

    /* Add game objects at a certain time step */
    spawnTimer += ctx.timeStep;
    if (spawnTimer > 1.0 && gameObjects < MAX_GAME_OBJECTS) {
        spawnTimer = 0.0;
        /* Randomize position and rotation */
        glm::vec3 pos = glm::vec3(Toolbox::genRandom(-50, 50.f), 1.f, Toolbox::genRandom(-50.f, 50.f));
        float rotation = Toolbox::genRandom(0.f, 360.f);
        float velocity = Toolbox::genRandom(15.f, 30.f);

        /* Create game object */
        Block *b = new Block(gameObjectMesh,
                             alive,         
                             pos,           
                             rotation,      
                             glm::vec3(2.f),         
                             velocity);     

        entities.push_back(b);
        blocks.push_back(b);
        spheres.push_back(&b->boundingSphere);
        gameObjects++;
        std::cout << "Game objects: " << gameObjects << std::endl;
    }

    player->update();
    for (auto block : blocks) {
        bool isHit = block->isHit;
        block->update(ctx.timeStep, 100.f, 100.f, player->boundingSphere, spheres);
        if (isHit != block->isHit) {
            std::cout << "Hit objects: " << ++hitObjects << std::endl;
        }
    }
}

void LabWorld::takeInput(Mouse &mouse, Keyboard &keyboard, const float timeStep) {
    if (mouse.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        camera->takeMouseInput(mouse.dx, mouse.dy);
    }
    else {
        camera->takeMouseInput(0.f, 0.f);
    }
    if (keyboard.isKeyPressed('w')) {
        camera->moveForward(timeStep);
        Toolbox::clamp(camera->position.x, -100.f, 100.f);
        Toolbox::clamp(camera->position.z, -100.f, 100.f);
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('a')) {
        camera->moveLeft(timeStep);
        Toolbox::clamp(camera->position.x, -100.f, 100.f);
        Toolbox::clamp(camera->position.z, -100.f, 100.f);
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('s')) {
        camera->moveBackward(timeStep);
        Toolbox::clamp(camera->position.x, -100.f, 100.f);
        Toolbox::clamp(camera->position.z, -100.f, 100.f);
        camera->position.y = 0.f;
    }
    if (keyboard.isKeyPressed('d')) {
        camera->moveRight(timeStep);
        Toolbox::clamp(camera->position.x, -100.f, 100.f);
        Toolbox::clamp(camera->position.z, -100.f, 100.f);
        camera->position.y = 0.f;
    }
}

void LabWorld::cleanUp() {

}