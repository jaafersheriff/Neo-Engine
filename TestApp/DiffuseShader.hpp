#pragma once

#include "DiffuseRenderable.hpp"
#include "System/RenderSystem/Shader.hpp"
#include "GLHelper.hpp"

using namespace neo;

class DiffuseShader : public Shader {

    public:
        DiffuseShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader(res, vert, frag)
        {}

        virtual void render(float dt, const CameraComponent *camera) override {
            bind();

            /* Load PV */
            loadMatrix(getUniform("P"), camera->getProj());
            loadMatrix(getUniform("V"), camera->getView());

            auto renderables = NeoEngine::getComponents<DiffuseRenderable>();
            for (auto dr : renderables) {
                /* Update should happen in some game-logic system before this */
                dr->update(dt);

                /* Bind mesh */
                const Mesh & mesh(*dr->mesh);
                glBindVertexArray(mesh.vaoId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId);

                loadMatrix(getUniform("M"), dr->M);

                /* DRAW */
                loadBool(getUniform("useOutline"), false);
                glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr);

                /* Draw wireframe */
                loadBool(getUniform("useOutline"), true);
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr);
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};