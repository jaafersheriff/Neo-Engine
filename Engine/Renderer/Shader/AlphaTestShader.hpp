#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

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

        virtual void render(const ECS& ecs) override {
            bind();

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderable : ecs.getComponentTuples<renderable::AlphaTestRenderable, MeshComponent, SpatialComponent>()) {
                auto spatial = renderable->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());

                /* Bind texture */
                loadTexture("diffuseMap", renderable->get<renderable::AlphaTestRenderable>()->mDiffuseMap);

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }
    };
}
