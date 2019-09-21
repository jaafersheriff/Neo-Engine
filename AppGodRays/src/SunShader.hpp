#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"

using namespace neo;

class SunShader : public Shader {

    public:

        SunShader(const std::string &vert, const std::string &frag) :
            Shader("Sun Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            glm::mat4 Vi = camera.getView();
            Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
            Vi = glm::transpose(Vi);
            loadUniform("Vi", Vi);

            auto& mesh = *Library::getMesh("plane");

            for (auto& renderable : Engine::getComponents<SunComponent>()) {

                /* Bind mesh */
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());

                loadUniform("center", renderable->getGameObject().getComponentByType<SpatialComponent>()->getPosition());

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
};
