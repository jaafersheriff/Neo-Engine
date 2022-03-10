#pragma once

#include "Renderer/Shader/Shader.hpp"
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
                auto camera = ecs.cGetComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", camera.get<CameraComponent>().getProj());
                loadUniform("V", camera.get<SpatialComponent>().getView());

                for (auto& tuple : ecs.getComponentTuples<renderable::WireframeRenderable, MeshComponent, SpatialComponent>()) {
                    const auto& [renderable, mesh, spatial] = tuple.get();

                    loadUniform("M", spatial.getModelMatrix());
                    loadUniform("wireColor", renderable.mColor);
                    mesh.mMesh->draw();
                }

                unbind();
            }
        };

}