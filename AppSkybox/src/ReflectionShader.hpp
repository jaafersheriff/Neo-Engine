#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "ReflectionComponent.hpp"
#include "SkyboxComponent.hpp"

using namespace neo;

class ReflectionShader : public Shader {

    public:

        ReflectionShader(const std::string &vert, const std::string &frag) :
            Shader("Reflection Shader", vert, frag)
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

            /* Load environment map */
            if (auto skybox = Engine::getComponentTuple<SkyboxComponent, CubeMapComponent>()) {
                loadUniform("cubeMap", skybox->get<CubeMapComponent>()->mTexture->mTextureID);
            }

            for (auto renderable : Engine::getComponentTuples<ReflectionComponent, MeshComponent, SpatialComponent>()) {

                /* Bind mesh */
                const Mesh & mesh(renderable.get<MeshComponent>()->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable.get<SpatialComponent>()->getModelMatrix());
                loadUniform("N", renderable.get<SpatialComponent>()->getNormalMatrix());

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
};