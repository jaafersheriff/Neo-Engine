#include "SkyboxShader.hpp"

#include "NeoEngine.hpp"

void SkyboxShader::render(float dt, const RenderSystem &renderSystem) {
    const auto cubes = renderSystem.getRenderables<SkyboxShader, CubeMapComponent>();
    if (!cubes.size()) {
        return;
    }

    bind();

    /* Load PV */
    const auto cameras = NeoEngine::getComponents<CameraComponent>();
    if (cameras.size()) {
        loadMatrix(getUniform("P"), cameras.at(0)->getProj());
        loadMatrix(getUniform("V"), cameras.at(0)->getView());
    }

    const auto cube = cubes[0];
    const Mesh *mesh = cube->getMesh();
    CHECK_GL(glBindVertexArray(mesh->vaoId));
    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

    /* Bind texture */
    const Texture *tex = cube->getTexture();
    loadInt(getUniform("cube"), tex->textureId);

    CHECK_GL(glDisable(GL_CULL_FACE));
    CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh->eleBufSize, GL_UNSIGNED_INT, nullptr));
    CHECK_GL(glEnable(GL_CULL_FACE));

    CHECK_GL(glBindVertexArray(0));
    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    unbind();
}