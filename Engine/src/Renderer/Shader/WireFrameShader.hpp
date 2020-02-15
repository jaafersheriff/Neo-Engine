#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

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

            virtual void render() {
                bind();
                CHECK_GL(glDisable(GL_CULL_FACE));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

                /* Load PV */
                auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());

                for (auto& renderable : Engine::getComponentTuples<renderable::WireframeRenderable, MeshComponent, SpatialComponent>()) {
                    const auto spatialComponent = renderable->get<SpatialComponent>();
                    loadUniform("M", spatialComponent->getModelMatrix());

                    glm::vec3 color(1.f);
                    if (const auto material = renderable->mGameObject.getComponentByType<MaterialComponent>()) {
                        color = material->material.diffuse;
                    }
                    loadUniform("wireColor", color);

                    /* Draw outline */
                    renderable->get<MeshComponent>()->getMesh().draw();
                }

                unbind();
            }
        };

}