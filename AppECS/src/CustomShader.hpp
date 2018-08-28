#pragma once

#include "CustomComponent.hpp"
#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"

using namespace neo;

class CustomShader : public Shader {

    public:
        CustomShader(RenderSystem &r, const std::string &vert, const std::string &frag) :
            Shader("Custom Shader", r.APP_SHADER_DIR, vert, frag)
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto r : renderSystem.getRenderables<CustomShader, RenderableComponent>()) {
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