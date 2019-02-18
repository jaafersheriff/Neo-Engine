#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "SkyboxComponent.hpp"

using namespace neo;

class SkyboxShader : public Shader {

    public:

        SkyboxShader(const std::string &vert, const std::string &frag) :
            Shader("Skybox Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            const auto cubes = MasterRenderer::getRenderables<SkyboxShader, SkyboxComponent>();
            if (!cubes.size()) {
                return;
            }

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            const auto cube = cubes[0];
            const Mesh & mesh = cube->getMesh();
            CHECK_GL(glBindVertexArray(mesh.vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

            /* Bind texture */
            loadUniform("cubeMap", cube->getGameObject().getComponentByType<CubeMapComponent>()->getTexture().mTextureID);

            /* Draw */
            mesh.draw();

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

            unbind();

            CHECK_GL(glEnable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LESS));
        }
};