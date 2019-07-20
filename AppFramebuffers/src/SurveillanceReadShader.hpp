#pragma once

#include "Engine.hpp"

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
            auto mesh(Library::getMesh("quad"));
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mElementBufferID));

            for (auto camera : Engine::getComponents<SurveillanceCamera>()) {
                loadUniform("M", camera->getGameObject().getSpatial()->getModelMatrix());

                /* Bind texture */
                camera->fbo->mTextures[0]->bind();
                loadUniform("fbo", camera->fbo->mTextures[0]->mTextureID);

                /* DRAW */
                mesh->draw();
            }

            unbind();
        }
};