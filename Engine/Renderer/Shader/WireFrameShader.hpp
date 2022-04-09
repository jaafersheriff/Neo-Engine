#pragma once

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    class WireframeShader : public Shader {

        public:
            WireframeShader() :
                Shader("Wire Shader",
                        R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        void main() {
                            gl_Position = P * V * M * vec4(vertPos, 1);
                        })",
                        R"(
                        uniform vec3 wireColor;
                        out vec4 color;
                        void main() {
                            color = vec4(wireColor, 1.0);
                        })"
                )
            {}

            virtual void render(const ECS& ecs) override {
                bind();
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Load PV */
                if (auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>()) {
                    auto&& [cameraEntity, _, cameraSpatial] = *cameraView;
                    loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                    loadUniform("V", cameraSpatial.getView());
                }

                for (const auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::WireframeRenderable, MeshComponent, SpatialComponent>().each()) {
                    loadUniform("M", spatial.getModelMatrix());
                    loadUniform("wireColor", renderable.mColor);
                    mesh.mMesh->draw();
                }

                unbind();
            }
        };

}