#include "DevWorld.hpp"

#include <iostream>

void DevWorld::init(Loader &loader) {
    /* Create entities */
    Entity *e = new Entity(loader.loadObjMesh("mr_krab.obj"),
                           loader.loadTexture("mr_krab.png"),
                           glm::vec3(15.f, 0.f, 0.f), 
                           glm::vec3(0), 
                           glm::vec3(10.f, 10.f, 10.f));
    entities.push_back(e);
    
    /* Billboards */
    Texture *cloudTexture = loader.loadTexture("cloud.png");
    for (int i = 0; i < 130; i++) {
        CloudBillboard *c = new CloudBillboard(
                        cloudTexture,
                        glm::vec3(
                            Toolbox::genRandom(-45.f, 75.f),
                            Toolbox::genRandom(-5.f, 5.f),
                            Toolbox::genRandom(-25.f, 25.f)),
                        glm::vec2(cloudTexture->width, cloudTexture->height)/75.f
                    );
        c->rotation = 360.f * Toolbox::genRandom();
        // cloudBoards.push_back(c);
    }

    sun = new Sun(glm::vec3(1.f), glm::vec3(1.f, 1.f, 0.f), 150, 250);

    /* Set up light */
    this->light = new Light;
    light->position = glm::vec3(-7, 1000, 1000);
    light->color = glm::vec3(1.f);
    light->attenuation = glm::vec3(1.f, 0.0f, 0.0f);

    /* Set up camera */
    this->camera = new Camera();
}

void DevWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateCloudRenderer(&cloudBoards);
    if (sun) {
        mr->activateSunRenderer(sun);
    }
    mr->activateEntityRenderer(&entities);
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
    if (sun) {
        sun->update(light);
        sun->innerColor = glm::vec3((1+glm::cos(glm::radians(ctx.runningTime*12.90)))/2, 
                                    (1+glm::cos(glm::radians(ctx.runningTime*37.98)))/2, 
                                    (1+glm::cos(glm::radians(ctx.runningTime*15.89)))/2);
        sun->outerColor = glm::vec3((1+glm::cos(glm::radians(ctx.runningTime*75.61)))/2, 
                                    (1+glm::cos(glm::radians(ctx.runningTime*67.89)))/2, 
                                    (1+glm::cos(glm::radians(ctx.runningTime*86.49)))/2);
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
        sun->updateInnerRadius(-5.f);
    }
    if (keyboard.isKeyPressed('x')) {
        sun->updateInnerRadius(5.f);
    }
    if (keyboard.isKeyPressed('c')) {
        sun->updateOuterRadius(-5.f);
    }
    if (keyboard.isKeyPressed('v')) {
        sun->updateOuterRadius(5.f);
    }
}

void DevWorld::cleanUp() {
    
}
