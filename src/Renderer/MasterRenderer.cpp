#include "MasterRenderer.hpp"

#include "World/World.hpp"
#include "Renderer/Renderer.hpp"

#include "Entity/EntityRenderer/EntityRenderer.hpp"

#include "Shader/GLSL.hpp"

#include "glm/gtc/matrix_transform.hpp"

MasterRenderer::MasterRenderer() {
}

/* Master render function */
void MasterRenderer::render(const Display &display, const World *world) {
    /* State setting */
    // TODO : why call this every frame? 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /* Reset rendering display */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    /* Create view matrix */
    const glm::mat4 v = glm::lookAt(world->camera.position, world->camera.lookAt, glm::vec3(0, 1, 0));

    /* Loop through active subrenderers */
    for (auto renderer = renderers.begin(); renderer != renderers.end(); renderer++) {
        /* Bind subrenderer's shader */
        (*renderer)->shader->bind();
        /* Do any subrenderer-specific prep */
        (*renderer)->prepare();
        /* Set global params */
        (*renderer)->setGlobals(&display.projectionMatrix, &v);
        /* Subrenderer render */
        (*renderer)->render(world);
        /* Unbind subrenderer's shader */
        (*renderer)->shader->unbind();
    }
}

void MasterRenderer::activateEntityRenderer(std::vector<Entity> *entities) {
    EntityRenderer *eR = new EntityRenderer;
    eR->activate(entities);
    renderers.push_back(eR);
}

void MasterRenderer::cleanUp() {
    /* Clean up all active subrenderers */
    for (auto renderer = renderers.begin(); renderer != renderers.end(); renderer++) {
        (*renderer)->cleanUp();
    }
}