#include "SkyWorld.hpp"

void SkyWorld::init(Loader &loader) {
    /* Camera */
    this->camera = new Camera();

    /* Main light source */
    this->light = new Light(glm::vec3(-1000, 1000, 1000), glm::vec3(1.f));

    /* Skybox */
    std::string textureNames[6] = {
        "arctic_ft.tga",
        "arctic_bk.tga",
        "arctic_up.tga",
        "arctic_dn.tga",
        "arctic_rt.tga",
        "arctic_lf.tga",

    };
    skybox = new Skybox(loader.loadCubeTexture(textureNames));

    /* Sun */
    sun = new Sun(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 0.f), 250, 150);

    /* Atmosphere */
    atmosphere = new Atmosphere(loader.loadObjMesh("geodisc.obj"), 
                                loader.loadTexture("atcolor.png"), 
                                loader.loadTexture("atglow.png"));
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