#pragma once

#include "Component/RenderableComponent/TexturedRenderable.hpp"
#include "Shader/Shader.hpp"

#include "Util/GLHelper.hpp"

#include "SkyboxComponent.hpp"

using namespace neo;

class SkyboxShader : public Shader {

    public:

        SkyboxShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Skybox Shader", res, vert, frag)
        {}

        virtual void render(float dt, const RenderSystem &renderSystem) override {
            const auto cubes = renderSystem.getRenderables<SkyboxShader, SkyboxComponent>();
            if (!cubes.size()) {
                return;
            }

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));
            bind();

            /* Load PV */
            const auto cameras = NeoEngine::getComponents<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                loadMatrix(getUniform("V"), cameras.at(0)->getView());
            }

            const auto cube = cubes[0];
            const Mesh & mesh = cube->getMesh();
            CHECK_GL(glBindVertexArray(mesh.vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

            /* Bind texture */
            const Texture & tex = cube->getTexture();
            loadInt(getUniform("cubeMap"), tex.textureId);

            CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

            unbind();

            CHECK_GL(glEnable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LESS));
        }
};