#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class AlphaTestShader : public Shader {

    public:

        AlphaTestShader() :
            Shader("AlphaTest Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                out vec2 fragTex;
                void main() {
                    fragTex = vertTex;
                    gl_Position = P * V * M * vec4(vertPos, 1.0);
                })",
                R"(
                #include "alphaDiscard.glsl"
                in vec2 fragTex;
                uniform sampler2D diffuseMap;
                out vec4 color;
                void main() {
                    vec4 albedo = texture(diffuseMap, fragTex);
                    alphaDiscard(albedo.a);
                    color = albedo;
                })")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (auto& renderable : Engine::getComponentTuples<renderable::AlphaTestRenderable, MeshComponent, SpatialComponent>()) {

                auto spatial = renderable->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());

                /* Bind texture */
                if (const auto diffuseMap = renderable->mGameObject.getComponentByType<DiffuseMapComponent>()) {
                    auto texture = (const Texture2D *)(diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("diffuseMap", texture->mTextureID);
                }

                /* DRAW */
                renderable->get<MeshComponent>()->getMesh().draw();
            }

            unbind();
        }
    };
}
