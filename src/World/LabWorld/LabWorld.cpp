#include "LabWorld.hpp"

#include "Block.hpp"

void LabWorld::init(Loader &loader) {
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