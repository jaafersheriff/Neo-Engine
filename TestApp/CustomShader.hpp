#pragma once

#include "CustomComponent.hpp"
#include "Shader/Shader.hpp"
#include "Shader/GLHelper.hpp"

using namespace neo;

class CustomShader : public Shader {

    public:
        CustomShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader(res, vert, frag)
        {}

        virtual std::string name() { return "Custom Shader"; }


        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), renderSystem.getCamera().getProj());
            loadMatrix(getUniform("V"), renderSystem.getCamera().getView());

            for (auto r : renderSystem.getRenderables<CustomShader, RenderableComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(*r->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                loadMatrix(getUniform("M"), r->getGameObject().getSpatial()->getModelMatrix());

                /* DRAW */
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};