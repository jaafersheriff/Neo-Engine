#include "MasterRenderer.hpp"

#include "World/World.hpp"
#include "Renderer/Renderer.hpp"

#include "Entity/EntityRenderer/EntityRenderer.hpp"
#include "Skybox/SkyboxRenderer/SkyboxRenderer.hpp"
#include "Cloud/CloudRenderer/CloudRenderer.hpp"

#include "Shader/GLSL.hpp"

#include "glm/gtc/matrix_transform.hpp"

MasterRenderer::MasterRenderer() {
}

/* Master render function */
void MasterRenderer::render(const Display &display, const World *world) {
    /* Reset rendering display */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    /* Create view matrix */
    const glm::mat4 v = glm::lookAt(world->camera.position, world->camera.lookAt, glm::vec3(0, 1, 0));

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

void MasterRenderer::init() {
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
}

void MasterRenderer::activateSkyboxRenderer(Skybox *sb) {
    SkyboxRenderer *sbR = new SkyboxRenderer;
    if (sbR->activate(sb)) {
        renderers.push_back(sbR);
    }
}

void MasterRenderer::activateCloudRenderer(std::vector<Billboard *> *billboards) {
    CloudRenderer *cR = new CloudRenderer;
    if (cR->activate(billboards)) {
        renderers.push_back(cR);
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
