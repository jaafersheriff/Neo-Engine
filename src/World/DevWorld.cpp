#include "DevWorld.hpp"

#include <iostream>

void DevWorld::init(Loader &loader) {
    /* Create entities */
    Entity *e = new Entity(loader.loadObjMesh("../resources/mr_krab.obj"),
                        //    loader.loadTexture("../resources/mr_krab.png"),
                           glm::vec3(15.f, 15.f, 0.f), 
                           glm::vec3(0), 
                           glm::vec3(10.f, 10, 10.f));
    // entities.push_back(e);

    /* Skybox */
    std::string textureNames[6] = {
                           "../resources/arctic_ft.tga", 
                           "../resources/arctic_bk.tga", 
                           "../resources/arctic_up.tga", 
                           "../resources/arctic_dn.tga", 
                           "../resources/arctic_rt.tga", 
                           "../resources/arctic_lf.tga"};
    // sb = new Skybox(loader.loadCubeTexture(textureNames));

    /* Billboards */
    Texture *cloudTexture = loader.loadTexture("../resources/cloud.png");
    for (int i = 0; i < 1; i++) {
        CloudBillboard *c = new CloudBillboard(
                        cloudTexture,
                        glm::vec3(
                            Toolbox::genRandom(25.f, 35.f),
                            Toolbox::genRandom(0.f, 10.f),
                            Toolbox::genRandom(-7.f, 7.f)),
                        glm::vec2(cloudTexture->width, cloudTexture->height)/75.f
                    );
        c->rotation = 360.f * Toolbox::genRandom();
        cloudBoards.push_back(c);
    }

    /* Sun */
    sun = new Sun(loader.loadTexture("moon.png"));

    /* Set up light */
    light.position = glm::vec3(-1000, 1000, 1000);
    light.color = glm::vec3(1.f);
    light.attenuation = glm::vec3(1.f, 0.0f, 0.0f);
}

void DevWorld::prepareRenderer(MasterRenderer *mr) {
    this->mr = mr;
    if (sb) {
        mr->activateSkyboxRenderer(sb);
    }
    mr->activateCloudRenderer(&cloudBoards);
    mr->activateEntityRenderer(&entities);
}

void DevWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    camera.update();
  
    if (isPaused) {
        return;
    }

    /* Update entities */
    for (auto entity : entities) {
        entity->update();
    }
    /* Update skybox */
    if (sb) {
        sb->update(ctx.displayTime);
    }
    /* Update cloudBoards */
    for (auto billboard : cloudBoards) {
        billboard->update(this->camera);
    }
}

void DevWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
    if (keyboard.isKeyPressed(' ')) {
        isPaused = !isPaused;
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
    if (keyboard.isKeyPressed('1')) {
        camera.updateLookAt(4.f, 0.f);
    }
    if (keyboard.isKeyPressed('2')) {
        camera.updateLookAt(-4.f, 0.f);
    }
    // TODO : put this in the GUI
    if (keyboard.isKeyPressed('m')) {
        mr->toggleWireFrameMode();
    }
    if (keyboard.isKeyPressed('~')) {
        // TODO : enable/disable GUI
    }
}

void DevWorld::cleanUp() {
    
}
