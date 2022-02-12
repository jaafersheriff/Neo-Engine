#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

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
                bind();

                CHECK_GL(glEnable(GL_LINE_SMOOTH));

                /* Load PV */
                auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());

                for (auto& line : ecs.getComponents<LineMeshComponent>()) {
                    glm::mat4 M(1.f);
                    if (line->mUseParentSpatial) {
                        if (auto spatial = line->getGameObject().getComponentByType<SpatialComponent>()) {
                            M = spatial->getModelMatrix();
                        }
                    }
                    loadUniform("M", M);

                    if (line->mWriteDepth) {
                        CHECK_GL(glEnable(GL_DEPTH_TEST));
                    }
                    else {
                        CHECK_GL(glDisable(GL_DEPTH_TEST));
                    }

                    /* Bind mesh */
                    line->getMesh().draw();
                }

                unbind();
            }
        };

}