#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class AlphaTestShader : public Shader {

    public:

        AlphaTestShader() :
            Shader("AlphaTest Shader",
                "#version 330 core\n\
                layout(location = 0) in vec3 vertPos;\
                layout(location = 2) in vec2 vertTex;\
                uniform mat4 P, V, M;\
                out vec2 fragTex;\
                void main() {\
                    fragTex = vertTex;\
                    gl_Position = P * V * M * vec4(vertPos, 1.0);\
                }",
                "#version 330 core\n\
                in vec2 fragTex;\
                uniform sampler2D diffuseMap;\
                out vec4 color;\
                void main() {\
                    vec4 albedo = texture(diffuseMap, fragTex);\
                    if (albedo.a < 0.1f) {\
                        discard;\
                    }\
                    color = albedo;\
                }")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& renderable : Engine::getComponents<renderable::AlphaTestRenderable>()) {
                auto meshComponent = renderable->getGameObject().getComponentByType<MeshComponent>();
                if (!meshComponent) {
                    continue;
                }

                /* Bind mesh */
                const Mesh & mesh(meshComponent->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable->getGameObject().getSpatial()->getModelMatrix());

                /* Bind texture */
                if (auto diffuseMap = renderable->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    auto texture = (const Texture2D *)(diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("diffuseMap", texture->mTextureID);
                }

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
}
