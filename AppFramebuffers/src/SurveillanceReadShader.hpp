#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"

#include "SurveillanceCamera.hpp"

using namespace neo;

class SurveillanceReadShader : public Shader {

    public:
        SurveillanceReadShader(const std::string &vert, const std::string &frag) :
            Shader("Surveillance Read", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            /* Bind mesh */
            auto mesh(Loader::getMesh("quad"));
            CHECK_GL(glBindVertexArray(mesh->mVAOID));

            for (auto camera : NeoEngine::getComponents<SurveillanceCamera>()) {
                loadUniform("M", camera->getGameObject().getSpatial()->getModelMatrix());

                /* Bind texture */
                camera->fbo->mTextures[0]->bind();
                loadUniform("fbo", camera->fbo->mTextures[0]->mTextureID);

                /* DRAW */
                mesh->draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
            unbind();
        }
};