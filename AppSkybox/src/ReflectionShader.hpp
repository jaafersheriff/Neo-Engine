#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"

#include "NeoEngine.hpp"

#include "ReflectionRenderable.hpp"
#include "SkyboxComponent.hpp"

using namespace neo;

class ReflectionShader : public Shader {

    public:

        ReflectionShader(const std::string &vert, const std::string &frag) :
            Shader("Reflection Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load environment map */
            loadUniform("cubeMap", NeoEngine::getComponents<SkyboxComponent>()[0]->getGameObject().getComponentByType<CubeMapComponent>()->getTexture().textureId);

            for (auto model : MasterRenderer::getRenderables<ReflectionShader, ReflectionRenderable>()) {
                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));

            unbind();
        }
};