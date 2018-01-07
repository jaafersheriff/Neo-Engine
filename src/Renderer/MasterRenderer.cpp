#include "MasterRenderer.hpp"

#include "World/World.hpp"
#include "Renderer/Renderer.hpp"

#include "Entity/EntityShader/EntityShader.hpp"
#include "Skybox/SkyboxShader/SkyboxShader.hpp"
#include "Cloud/CloudShader/CloudShader.hpp"
#include "Sun/SunShader/SunShader.hpp"
#include "Atmosphere/AtmosphereShader/AtmosphereShader.hpp"

#include "Shader/GLSL.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

MasterRenderer::MasterRenderer() {
}

/* Master render function */
void MasterRenderer::render(const Display &display, const World *world) {
    /* Reset rendering display */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    /* Create view matrix */
    const glm::mat4 v = glm::lookAt(world->camera->position, world->camera->lookAt, glm::vec3(0, 1, 0));

    /* Loop through active subrenderers */
    for (auto &shader : shaders) {
        /* Bind subrenderer's shader */
        shader->bind();
        /* Set global params */
        shader->setGlobals(&display.projectionMatrix, &v);
        /* Subrenderer render */
        shader->render(world);
        /* Unbind subrenderer's shader */
        shader->unbind();
    }
}

void MasterRenderer::init(const Context &ctx) {
    this->verbose = ctx.verbose;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MasterRenderer::activateEntityShader(std::vector<Entity *> *entities) {
    EntityShader *eShader = new EntityShader;
    if (eShader->init(entities)) {
        shaders.push_back(eShader);
    }
    else {
        delete eShader;
        eShader = nullptr;
    }
    if (verbose) {
        if (eShader) {
            std::cout << "Entity Shader activated" << std::endl;
        }
        else {
            std::cout << "Entity Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSkyboxShader(Skybox *sb) {
    SkyboxShader *sShader = new SkyboxShader;
    if (sShader->init(sb)) {
        shaders.push_back(sShader);
    }
    else {
        delete sShader;
        sShader = nullptr;
    }
    if (verbose) {
        if (sShader) {
            std::cout << "Skybox Shader activated" << std::endl;
        }
        else {
            std::cout << "Skybox Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateCloudShader(std::vector<CloudBillboard *> *billboards) {
    CloudShader *cShader = new CloudShader;
    if (cShader->init(billboards)) {
        shaders.push_back(cShader);
    }
    else {
        delete cShader;
        cShader = nullptr;
    }
    if (verbose) {
        if (cShader) {
            std::cout << "Cloud Shader activated" << std::endl;
        }
        else {
            std::cout << "Cloud Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSunShader(Sun *sun) {
    SunShader *sShader = new SunShader;
    if (sShader->init(sun)) {
        shaders.push_back(sShader);
    }
    else {
        delete sShader;
        sShader = nullptr;
    }
    if (verbose) {
        if (sShader) {
            std::cout << "Sun Shader activated" << std::endl;
        }
        else {
            std::cout << "Sun Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateAtmosphereShader(Atmosphere *atm) {
    AtmosphereShader *aShader = new AtmosphereShader;
    if (aShader->init(atm)) {
        shaders.push_back(aShader);
    }
    else {
        delete aShader;
        aShader = nullptr;
    }
    if (verbose) {
        if (aShader) {
            std::cout << "Atmosphere Shader activated" << std::endl;
        }
        else {
            std::cout << "Atmosphere Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::cleanUp() {
    /* Clean up all active subrenderers */
    for (auto &shader : shaders) {
        shader->cleanUp();
    }
}

void MasterRenderer::toggleWireFrameMode() {
    wireFrame = !wireFrame;
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
}
