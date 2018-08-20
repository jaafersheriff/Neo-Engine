#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"

#include "SurveillanceCamera.hpp"

using namespace neo;

class SurveillanceReadShader : public Shader {

    public:
        SurveillanceReadShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Surveillance Read", res, vert, frag)
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), camera.getProj());
            loadMatrix(getUniform("V"), camera.getView());

            /* Bind mesh */
            auto mesh(Loader::getMesh("quad"));
            CHECK_GL(glBindVertexArray(mesh->vaoId));

            for (auto camera : NeoEngine::getComponents<SurveillanceCamera>()) {
                loadMatrix(getUniform("M"), camera->getGameObject().getSpatial()->getModelMatrix());

                /* Bind texture */
                const Texture & texture(*camera->fboTex);
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.textureId));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.textureId));
                loadInt(getUniform("fbo"), texture.textureId);

                /* DRAW */
                CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->vertBufSize));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
            unbind();
        }
};