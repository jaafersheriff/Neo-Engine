#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/OutlineRenderable.hpp"

#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

#include <tracy/TracyOpenGL.hpp>

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
            ZoneScoped;
            TracyGpuZone("OutlineShader");
            bind();

            glCullFace(GL_FRONT);

            /* Load PV */
            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            NEO_ASSERT(cameraView, "No main camera :(");
            auto&& [cameraEntity, __, cameraSpatial] = *cameraView;
            loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
            loadUniform("V", cameraSpatial.getView());

            const auto& viewport = ecs.cGetComponent<ViewportDetailsComponent>();
            loadUniform("screenSize", glm::vec2(std::get<1>(*viewport).mSize));

            for (auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::OutlineRenderable, MeshComponent, SpatialComponent>().each()) {

                // VFC
                if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                    if (!culled->isInView(ecs, entity, cameraEntity)) {
                        continue;
                    }
                }

                // Match the transforms of spatial component..
                loadUniform("M", spatial.getModelMatrix());
                loadUniform("N", spatial.getNormalMatrix());

                loadUniform("width", renderable.mScale);
                loadUniform("outlineColor", renderable.mColor);

                /* DRAW */
                mesh.mMesh->draw();
            }

            unbind();
        }
    };
}
