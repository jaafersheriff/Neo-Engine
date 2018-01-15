#include "CloudWorld.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

void CloudWorld::init(Context &ctx, Loader &loader) {
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

void CloudWorld::prepareRenderer(MasterRenderer *mr) {
    mr->activateSunShader(sun);
    mr->activateCloudShader(&cloudBoards);
}

void CloudWorld::prepareUniforms() {
    UniformData *PData = new UniformData{ UniformType::Mat4, "P", (void *)&P };
    UniformData *VData = new UniformData{ UniformType::Mat4, "V", (void *)&V };
    UniformData *cameraPos = new UniformData{ UniformType::Vec3, "cameraPos", (void *)&camera->position };
    UniformData *lightPos = new UniformData{ UniformType::Vec3, "lightPos", (void *)&light->position };
    UniformData *lightCol = new UniformData{ UniformType::Vec3, "lightCol", (void *)&light->color };
    UniformData *lightAtt = new UniformData{ UniformType::Vec3, "lightAtt", (void *)&light->attenuation };
    
    uniforms[MasterRenderer::ShaderTypes::SUN_SHADER].push_back(PData);
    uniforms[MasterRenderer::ShaderTypes::SUN_SHADER].push_back(VData);

    uniforms[MasterRenderer::ShaderTypes::CLOUD_SHADER].push_back(PData);
    uniforms[MasterRenderer::ShaderTypes::CLOUD_SHADER].push_back(VData);
    uniforms[MasterRenderer::ShaderTypes::CLOUD_SHADER].push_back(lightPos);
    uniforms[MasterRenderer::ShaderTypes::CLOUD_SHADER].push_back(lightCol);
}

void CloudWorld::update(Context &ctx) {
    this->V = glm::lookAt(camera->position, camera->lookAt, glm::vec3(0, 1, 0));
    takeInput(ctx.mouse, ctx.keyboard);
    camera->update();
  
    /* Update cloudBoards */
    for (auto billboard : cloudBoards) {
        billboard->update(this->camera);
    }
    /* Update sun */
    if (sun) {
        sun->update();
    }
}

void CloudWorld::takeInput(Mouse &mouse, Keyboard &keyboard) {
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

void CloudWorld::cleanUp() {
    
}

