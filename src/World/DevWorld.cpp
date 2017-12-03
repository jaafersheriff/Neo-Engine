#include "DevWorld.hpp"

void DevWorld::init(Loader &loader) {
    /* Create entities */
    Entity *e = new Entity(loader.loadObjMesh("../resources/mr_krab.obj"),
                      loader.loadTexture("../resources/mr_krab.png"),
                      glm::vec3(15.f, 0.f, 0.f), 
                      glm::vec3(0), 
                      glm::vec3(10.f, 10, 10.f));
    e->texture.diffuseColor = glm::vec3(0.77f, 0.1f, 1.f);
    entities.push_back(e);

    /* Skybox */
    std::string textureNames[6] = {
                           "../resources/arctic_ft.tga", 
                           "../resources/arctic_bk.tga", 
                           "../resources/arctic_up.tga", 
                           "../resources/arctic_dn.tga", 
                           "../resources/arctic_rt.tga", 
                           "../resources/arctic_lf.tga"};
    sb = new Skybox(loader.loadCubeTexture(textureNames));

    /* Billboards */
    billboards.push_back(new Billboard(
                             loader.loadTexture("../resources/Tatooine.jpg"), 
                             glm::vec3(0.f), 
                             glm::vec2(10.f, 10.f)));
        
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
    mr->activateEntityRenderer(entities);
    mr->activateBillboardRenderer(billboards);
}

void DevWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    camera.update();
    
    if (isPaused) {
        return;
    }

    for (unsigned int i = 0; i < entities.size(); i++) {
        entities[i]->update();
    }
    if (sb) {
        sb->update(ctx.displayTime);
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
