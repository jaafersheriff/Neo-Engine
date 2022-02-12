#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

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
                CHECK_GL(glDisable(GL_CULL_FACE));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

                /* Load PV */
                auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());

                for (auto& renderableIt : ecs.getComponentTuples<renderable::WireframeRenderable, MeshComponent, SpatialComponent>()) {
                    const auto renderable = renderableIt->get<renderable::WireframeRenderable>();
                    const auto spatialComponent = renderableIt->get<SpatialComponent>();

                    loadUniform("M", spatialComponent->getModelMatrix());

                    loadUniform("wireColor", renderable->mColor);

                    /* Draw outline */
                    renderableIt->get<MeshComponent>()->mMesh.draw();
                }

                unbind();
            }
        };

}