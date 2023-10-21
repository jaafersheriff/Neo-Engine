#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <glm/gtc/matrix_transform.hpp>

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
                TRACY_GPUN("WireframeShader");
                bind();
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                /* Load PV */
                if (auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>()) {
                    auto&& [cameraEntity, _, cameraSpatial] = *cameraView;
                    loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                    loadUniform("V", cameraSpatial.getView());
                }

                for (const auto&& [entity, renderable, bb, spatial] : ecs.getView<renderable::WireframeRenderable, BoundingBoxComponent, SpatialComponent>().each()) {
                    glm::mat4 M = glm::scale(glm::mat4(1.f), (bb.mMax - bb.mMin) / 2.f);
                    glm::translate(M, spatial.getPosition());
                    loadUniform("M", M);
                    loadUniform("wireColor", renderable.mColor);
                    Library::getMesh("cube").mMesh->draw();
                }

                unbind();
            }
        };

}