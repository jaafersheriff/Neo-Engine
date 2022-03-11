#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/RenderableComponent/AlphaTestRenderable.hpp"

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
            const auto& camera = ecs.getView<MainCameraComponent, SpatialComponent>();
            NEO_ASSERT(camera.size_hint() == 1, "No main camera exists");
            loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(camera.front())->getProj());
            loadUniform("V", camera.get<const SpatialComponent>(camera.front()).getView());

            for (auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::AlphaTestRenderable, MeshComponent, SpatialComponent>().each()) {
                loadUniform("M", spatial.getModelMatrix());

                /* Bind texture */
                loadTexture("diffuseMap", *renderable.mDiffuseMap);

                /* DRAW */
                mesh.mMesh->draw();
            }
            unbind();
        }
    };
}
