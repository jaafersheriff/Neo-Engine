#include "MasterRenderer.hpp"

#include "World/World.hpp"
#include "Renderer/Renderer.hpp"

#include "Entity/EntityRenderer/EntityRenderer.hpp"
#include "Skybox/SkyboxRenderer/SkyboxRenderer.hpp"
#include "Cloud/CloudRenderer/CloudRenderer.hpp"
#include "Sun/SunRenderer/SunRenderer.hpp"
#include "Atmosphere/AtmosphereRenderer/AtmosphereRenderer.hpp"

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
    for (auto &renderer : renderers) {
        /* Bind subrenderer's shader */
        renderer->shader->bind();
        /* Do any subrenderer-specific prep */
        renderer->prepare();
        /* Set global params */
        renderer->setGlobals(&display.projectionMatrix, &v);
        /* Subrenderer render */
        renderer->render(world);
        /* Unbind subrenderer's shader */
        renderer->shader->unbind();
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

void MasterRenderer::activateEntityRenderer(std::vector<Entity *> *entities) {
    EntityRenderer *eR = new EntityRenderer;
    if (eR->activate(entities)) {
        renderers.push_back(eR);
    }
    else {
        delete eR;
        eR = nullptr;
    }
    if (verbose) {
        if (eR) {
            std::cout << "Entity Renderer activated" << std::endl;
        }
        else {
            std::cout << "Entity Renderer failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSkyboxRenderer(Skybox *sb) {
    SkyboxRenderer *sbR = new SkyboxRenderer;
    if (sbR->activate(sb)) {
        renderers.push_back(sbR);
    }
    else {
        delete sbR;
        sbR = nullptr;
    }
    if (verbose) {
        if (sbR) {
            std::cout << "Skybox Renderer activated" << std::endl;
        }
        else {
            std::cout << "Skybox Renderer failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateCloudRenderer(std::vector<CloudBillboard *> *billboards) {
    CloudRenderer *cR = new CloudRenderer;
    if (cR->activate(billboards)) {
        renderers.push_back(cR);
    }
    else {
        delete cR;
        cR = nullptr;
    }
    if (verbose) {
        if (cR) {
            std::cout << "Cloud Renderer activated" << std::endl;
        }
        else {
            std::cout << "Cloud Renderer failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSunRenderer(Sun *sun) {
    SunRenderer *sR = new SunRenderer;
    if (sR->activate(sun)) {
        renderers.push_back(sR);
    }
    else {
        delete sR;
        sR = nullptr;
    }
    if (verbose) {
        if (sR) {
            std::cout << "Sun Renderer activated" << std::endl;
        }
        else {
            std::cout << "Sun Renderer failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateAtmosphereRenderer(Atmosphere *atm) {
    AtmosphereRenderer *aR = new AtmosphereRenderer;
    if (aR->activate(atm)) {
        renderers.push_back(aR);
    }
    else {
        delete aR;
        aR = nullptr;
    }
    if (verbose) {
        if (aR) {
            std::cout << "Atmosphere Renderer activated" << std::endl;
        }
        else {
            std::cout << "Atmosphere Renderer failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::cleanUp() {
    /* Clean up all active subrenderers */
    for (auto &renderer : renderers) {
        renderer->cleanUp();
    }
}

void MasterRenderer::toggleWireFrameMode() {
    wireFrame = !wireFrame;
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
}
