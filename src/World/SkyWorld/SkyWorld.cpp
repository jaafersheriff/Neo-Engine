#include "SkyWorld.hpp"

void SkyWorld::init(Loader &loader) {
    /* Camera */
    this->camera = new Camera();

    /* Main light source */
    this->light = new Light(glm::vec3(-1000, 1000, 1000), glm::vec3(1.f));

    /* Skybox */
    std::string textureNames[6] = {
        "../resources/arctic_ft.tga",
        "../resources/arctic_bk.tga",
        "../resources/arctic_up.tga",
        "../resources/arctic_dn.tga",
        "../resources/arctic_rt.tga",
        "../resources/arctic_lf.tga",

    };
    skybox = new Skybox(loader.loadCubeTexture(textureNames));

    /* Sun */
    sun = new Sun(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 0.f), 250, 150);

    /* Atmosphere */
    atmosphere = new Atmosphere(loader.loadObjMesh("../resources/geodisc.obj"), 
                                loader.loadTexture("../resources/atcolor.png"), 
                                loader.loadTexture("../resources/atglow.png"));
}

void SkyWorld::prepareRenderer(MasterRenderer *mr) {
    if(skybox) {
        mr->activateSkyboxRenderer(skybox);
    }
    if (sun) {
        mr->activateSunRenderer(sun);
    }
    if (atmosphere) {
        mr->activateAtmosphereRenderer(atmosphere);
    }
}

void SkyWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    camera->update();

    if (skybox) {
        skybox->update(ctx.displayTime);
    }
}

void SkyWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
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

void SkyWorld::cleanUp() {

}