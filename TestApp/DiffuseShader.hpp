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
                dr->update(dt);

                /* Bind mesh */
                const Mesh & mesh(*dr->mesh);
                glBindVertexArray(mesh.vaoId);

                loadMatrix(getUniform("M"), dr->M);

                /* Bind vertex buffer VBO */
                int pos = getAttribute("vertPos");
                glEnableVertexAttribArray(pos);
                glBindBuffer(GL_ARRAY_BUFFER, mesh.vertBufId);
                glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

                /* Bind normal buffer VBO */
                pos = getAttribute("vertNor");
                if (pos != -1 && mesh.norBufId != 0) {
                    glEnableVertexAttribArray(pos);
                    glBindBuffer(GL_ARRAY_BUFFER, mesh.norBufId);
                    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
                }

                /* Bind texture coordinate buffer VBO */
                pos = getAttribute("vertTex");
                if (pos != -1 && mesh.texBufId != 0) {
                    glEnableVertexAttribArray(pos);
                    glBindBuffer(GL_ARRAY_BUFFER, mesh.texBufId);
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
                }

                /* Bind indices buffer VBO */
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId);

                /* DRAW */
                glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr);

                /* Unload mesh */
                glDisableVertexAttribArray(getAttribute("vertPos"));
                pos = getAttribute("vertNor");
                if (pos != -1) {
                    glDisableVertexAttribArray(pos);
                }
                pos = getAttribute("vertTex");
                if (pos != -1) {
                    glDisableVertexAttribArray(pos);
                }
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};