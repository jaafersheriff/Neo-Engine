#include "DevWorld.hpp"

#include <iostream>

void DevWorld::init(Loader &loader) {
    /* Set up light */
    this->light = new Light(glm::vec3(-1000, 1000, 1000), glm::vec3(1.f), glm::vec3(1.f, 0.0f, 0.0f));

    /* Set up camera */
    this->camera = new Camera;

    /* Billboards */
    Texture *cloudTexture = loader.loadTexture("cloud.png");
    for (int i = 0; i < 50; i++) {
        CloudBillboard *c = new CloudBillboard(
                        cloudTexture,
                        glm::vec3(
                            Toolbox::genRandom(-15.f, 15.f),
                            Toolbox::genRandom(-5.f, 5.f),
                            Toolbox::genRandom(-25.f, 25.f)),
                        glm::vec2(cloudTexture->width, cloudTexture->height)/75.f);
        c->rotation = 360.f * Toolbox::genRandom();
        cloudBoards.push_back(c);
    }

    /* Sun */    
    sun = new Sun(this->light, glm::vec3(1.f), glm::vec3(1.f, 1.f, 0.f), 75, 150);
}

void DevWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateSunShader(sun);
    mr->activateCloudShader(&cloudBoards);
    mr->activateEntityShader(&entities);
}

void DevWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    gameTime = ctx.displayTime;
    camera->update();
  
    if (isPaused) {
        return;
    }

    /* Update entities */
    for (auto entity : entities) {
        entity->update();
    }
    /* Update cloudBoards */
    for (auto billboard : cloudBoards) {
        billboard->update(this->camera);
    }
    /* Update sun */
    if (sun) {
        sun->update();
    }
}

void DevWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
    if (keyboard.isKeyPressed(' ')) {
        isPaused = !isPaused;
    }

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
    if (keyboard.isKeyPressed('~')) {
        // TODO : enable/disable GUI
    }
    if (keyboard.isKeyPressed('z')) {
        light->position.x += 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
    if (keyboard.isKeyPressed('x')) {
        light->position.x -= 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
    if (keyboard.isKeyPressed('c')) {
        light->position.y += 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
    if (keyboard.isKeyPressed('v')) {
        light->position.y -= 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
    if (keyboard.isKeyPressed('b')) {
        light->position.z += 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
    if (keyboard.isKeyPressed('n')) {
        light->position.z -= 35.f;
        std::cout << Toolbox::vectorToString(light->position) << std::endl;
    }
}

void DevWorld::cleanUp() {
    
}
