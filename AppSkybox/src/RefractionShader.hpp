#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "NeoEngine.hpp"

#include "RefractionComponent.hpp"
#include "SkyboxComponent.hpp"

using namespace neo;

class RefractionShader : public Shader {

    public:

        RefractionShader(const std::string &vert, const std::string &frag) :
            Shader("Refraction Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load environment map */
            if (auto skybox = NeoEngine::getSingleComponent<SkyboxComponent>()) {
                if (auto cubemap = skybox->getGameObject().getComponentByType<CubeMapComponent>()) {
                    loadUniform("cubeMap", cubemap->mTexture->mTextureID);
                }
            }

            for (auto& renderable : NeoEngine::getComponents<RefractionComponent>()) {
                auto model = renderable->getGameObject().getComponentByType<MeshComponent>();
                if (!model) {
                    continue;
                }

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());
                loadUniform("ratio", renderable->ratio);

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