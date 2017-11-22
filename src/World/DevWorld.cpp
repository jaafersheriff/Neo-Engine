#include "DevWorld.hpp"

void DevWorld::init(Loader &loader) {
    // Load Meshes/Textures
    Mesh *mesh = loader.loadObjMesh("../resources/bunny.obj");
    Texture texture = loader.loadTexture("../resources/falcon.jpg");

    // Create entities
    Entity e = Entity(mesh, texture, glm::vec3(5.f, 0.f, 0.f), glm::vec3(0), glm::vec3(10.f, 10, 10.f));
    e.texture.diffuseColor = glm::vec3(2.f, 0.f, 0.f);
    entities.push_back(e);

    // Set up light
    light.position = glm::vec3(-1000, 1000, 1000);
    light.color = glm::vec3(1.f);
    light.attenuation = glm::vec3(1.f, 0.0f, 0.0f);
}

void DevWorld::prepareRenderer(MasterRenderer *mr) {
    this->mr = mr;
    mr->activateEntityRenderer(&entities);
}

void DevWorld::update(Context &ctx) {
    takeInput(ctx.mouse, ctx.keyboard);
    if (isPaused) {
        return;
    }

    camera.update();

    for (unsigned int i = 0; i < entities.size(); i++) {
        entities[i].update();
    }
}

void DevWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
    if (keyboard.isKeyPressed(' ')) {
        isPaused = !isPaused;
    }
    if (isPaused) {
        return;
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
