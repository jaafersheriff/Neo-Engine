#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <tracy/TracyOpenGL.hpp>

namespace neo {

    class LineShader : public Shader {

        public:
            LineShader() :
                Shader("Line Shader",
                    R"(
                    layout (location = 0) in vec3 vertPos;
                    layout (location = 1) in vec3 vertColor;
                    uniform mat4 P, V, M;
                    out vec3 vCol;
                    void main() {
                        gl_Position = P * V * M * vec4(vertPos, 1);
                        vCol = vertColor;
                    })",
                    R"(
                    in vec3 vCol;
                    out vec4 color;
                    void main() {
                        color = vec4(vCol, 1.0);
                    })"
                )
            {}

            virtual void render(const ECS& ecs) override {
                TRACY_GPUN("LineShader");
                bind();

                glEnable(GL_LINE_SMOOTH);

                /* Load PV */
                if (auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>()) {
                    auto&& [cameraEntity, _, cameraSpatial] = *cameraView;
                    loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                    loadUniform("V", cameraSpatial.getView());
                }

                ecs.getView<LineMeshComponent>().each([this, &ecs](ECS::Entity entity, const LineMeshComponent& line) {
                    glm::mat4 M(1.f);
                    if (line.mUseParentSpatial) {
                        if (auto spatial = ecs.cGetComponent<SpatialComponent>(entity)) {
                            M = spatial->getModelMatrix();
                        }
                    }
                    loadUniform("M", M);

                    if (line.mWriteDepth) {
                        glEnable(GL_DEPTH_TEST);
                    }
                    else {
                        glDisable(GL_DEPTH_TEST);
                    }

                    /* Bind mesh */
                    line.getMesh().draw();
                    });

                unbind();
            }
        };

}
