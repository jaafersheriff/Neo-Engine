#include "DevWorld.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

void DevWorld::init(Context &ctx, Loader &loader) {
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

    /* World-specific members */
    this->P = ctx.display.projectionMatrix;
    this->V = glm::lookAt(camera->position, camera->lookAt, glm::vec3(0, 1, 0));
}

void DevWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateSunShader(sun);
    mr->activateCloudShader(&cloudBoards);
    mr->activateEntityShader(&entities);
}

void DevWorld::prepareUniforms() {
    UniformData *PData = new UniformData{ UniformType::Mat4, "P", (void *)&P };
    UniformData *VData = new UniformData{ UniformType::Mat4, "V", (void *)&V };
    UniformData *cameraPos = new UniformData{ UniformType::Vec3, "cameraPos", (void *)&camera->position };
    UniformData *lightPos = new UniformData{ UniformType::Vec3, "lightPos", (void *)&light->position };
    UniformData *lightCol = new UniformData{ UniformType::Vec3, "lightCol", (void *)&light->color };
    UniformData *lightAtt = new UniformData{ UniformType::Vec3, "lightAtt", (void *)&light->attenuation };
    
    std::vector<UniformData *> sunData;
    sunData.push_back(PData);
    sunData.push_back(VData);
    uniforms[MasterRenderer::ShaderTypes::SUN_SHADER] = sunData;

    std::vector<UniformData *> cloudData;
    cloudData.push_back(PData);
    cloudData.push_back(VData);
    cloudData.push_back(lightPos);
    cloudData.push_back(lightCol);
    uniforms[MasterRenderer::ShaderTypes::CLOUD_SHADER] = cloudData;

    std::vector<UniformData *> entityData;
    entityData.push_back(PData);
    entityData.push_back(VData);
    entityData.push_back(lightPos);
    entityData.push_back(lightCol);
    entityData.push_back(lightAtt);
    uniforms[MasterRenderer::ShaderTypes::ENTITY_SHADER] = entityData;
}

void DevWorld::update(Context &ctx) {
    this->V = glm::lookAt(camera->position, camera->lookAt, glm::vec3(0, 1, 0));
    takeInput(ctx.mouse, ctx.keyboard);
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
