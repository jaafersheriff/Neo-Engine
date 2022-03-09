#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/OutlineRenderable.hpp"

namespace neo {

    class OutlineShader : public Shader {

    public:

        float mWidth = 2.f;

        OutlineShader() :
            Shader("Outline Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 1) in vec3 vertNor;
                uniform mat4 P, V, M;
                uniform mat3 N;
                uniform float width;
                uniform vec2 screenSize;
                void main() {
                    vec4 clipPos = P * V * M * vec4(vertPos, 1.0);
                    vec3 clipNor = mat3(P * V) * N * vertNor;
                    vec2 offset = normalize(clipNor.xy) / screenSize * width * clipPos.w * 2;
                    clipPos += vec4(offset, 0.0, 0.0);
                    gl_Position = clipPos;
                })",
                R"(
                uniform vec4 outlineColor;
                out vec4 color;
                void main() {
                    color = outlineColor;
                })")
        {}

        virtual void render(const ECS& ecs) override {
            bind();

            glCullFace(GL_FRONT);

            /* Load PV */
            auto camera = ecs.cGetComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera.get<CameraComponent>().getProj());
            loadUniform("V", camera.get<CameraComponent>().getView());
            auto viewport = ecs.getComponent<ViewportDetailsComponent>();
            loadUniform("screenSize", glm::vec2(viewport->mSize));

            const auto cameraFrustum = ecs.getComponent<FrustumComponent>(camera.mEntity);

            for (auto& tuple : ecs.getComponentTuples<renderable::OutlineRenderable, MeshComponent, SpatialComponent>()) {
                auto&& [renderable, mesh, spatial] = tuple.raw();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("OutlineShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.getComponent<BoundingBoxComponent>(tuple.mEntity)) {
                        if (!cameraFrustum->isInFrustum(*spatial, *boundingBox)) {
                            continue;
                        }
                    }
                }

                // Match the transforms of spatial component..
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("N", spatial->getNormalMatrix());

                loadUniform("width", renderable->mScale);
                loadUniform("outlineColor", renderable->mColor);

                /* DRAW */
                mesh->mMesh.draw();
            }

            unbind();
        }
    };
}
