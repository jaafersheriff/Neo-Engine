#include "LabWorld.hpp"

#include "Block.hpp"

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
}

void LabWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateEntityShader(&entities);
}

void LabWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    camera->update();

    /* Add game objects at a certain time step */
    if (!((int) ctx.runningTime % 5) && gameObjects < MAX_GAME_OBJECTS) {
        /* Randomize position*/
        glm::vec3 pos = glm::vec3(Toolbox::genRandom(100.f), 0.f, Toolbox::genRandom(100.f));
        float rotation = Toolbox::genRandom(0.f, 360.f);
        entities.push_back(new Block(loader->loadObjMesh("cube.obj"),   /* Mesh */
                                     pos,                               /* Position */
                                     glm::vec3(0.f, rotation, 0.f),     /* Rotation */
                                     glm::vec3(10.f),                   /* Scale */
                                     Toolbox::genRandom(10.f, 20.f)));   /* Velocity */
        gameObjects++;
    }

    for (auto b : entities) {

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
    }
    if (keyboard.isKeyPressed('a')) {
        camera->moveLeft();
    }
    if (keyboard.isKeyPressed('s')) {
        camera->moveBackward();
    }
    if (keyboard.isKeyPressed('d')) {
        camera->moveRight();
    }
    if (keyboard.isKeyPressed('e')) {
        camera->moveDown();
    }
    if (keyboard.isKeyPressed('r')) {
        camera->moveUp();
    }
}

void LabWorld::cleanUp() {

}