#pragma once

#include "NeoEngine.hpp"

#include "DiffuseRenderable.hpp"
#include "Shader/Shader.hpp"
#include "Shader/GlHelper.hpp"

using namespace neo;

class DiffuseShader : public Shader {

    public: 
    
        DiffuseShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader("Diffuse Shader", res, vert, frag) 
        {}

        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            /* Load PV */
            auto cameras = NeoEngine::getComponents<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                loadMatrix(getUniform("V"), cameras.at(0)->getView());
                loadVector(getUniform("camPos"), cameras.at(0)->getGameObject().getSpatial()->getPosition());
            }

            /* Load light */
            auto lights = NeoEngine::getComponents<LightComponent>();
            if (lights.size()) {
                loadVector(getUniform("lightPos"), lights.at(0)->getGameObject().getSpatial()->getPosition());
            }

            for (auto diffuse : renderSystem.getRenderables<DiffuseShader, DiffuseRenderable>()) {
                loadMatrix(getUniform("M"), diffuse->getGameObject().getSpatial()->getModelMatrix());
                loadMatrix(getUniform("N"), diffuse->getGameObject().getSpatial()->getNormalMatrix());

                /* Bind mesh */
                const Mesh * mesh(diffuse->getMesh());
                CHECK_GL(glBindVertexArray(mesh->vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

                /* Bind texture */
                // TODO


                /* DRAW */
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh->eleBufSize, GL_UNSIGNED_INT, nullptr));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            // TODO : clean up texture
            unbind();
        }
};
