#pragma once

#include "CustomComponent.hpp"
#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class CustomShader : public Shader {

    public:
        CustomShader(const std::string &vert, const std::string &frag) :
            Shader("Custom Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto r : MasterRenderer::getRenderables<CustomShader, RenderableComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(r->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                loadUniform("M", r->getGameObject().getSpatial()->getModelMatrix());

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
        }
};