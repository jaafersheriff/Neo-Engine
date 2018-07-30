#pragma once

#include "CustomComponent.hpp"
#include "Shader/Shader.hpp"
#include "Shader/GLHelper.hpp"

using namespace neo;

class WireShader : public Shader {

    public:
        WireShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Wire Shader", res, vert, frag)
        {}
        
        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), renderSystem.getCamera().getProj());
            loadMatrix(getUniform("V"), renderSystem.getCamera().getView());

            for (auto r : renderSystem.getRenderables<WireShader, RenderableComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(*r->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                loadMatrix(getUniform("M"), r->getGameObject().getSpatial()->getModelMatrix());

                /* Draw outline */
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};