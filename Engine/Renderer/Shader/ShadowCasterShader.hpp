#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/ShadowCasterRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    class ShadowCasterShader : public Shader {

        public:

            ShadowCasterShader(const int dimension) :
                Shader("Shadow Caster",
                    R"(
                        layout (location = 0) in vec3 vertPos;
                        layout (location = 2) in vec2 vertTex;
                        uniform mat4 P, V, M;
                        out vec2 fragTex;
                        void main() { gl_Position = P * V * M * vec4(vertPos, 1); fragTex = vertTex; })",
                    R"(
                        #include "alphaDiscard.glsl"
                        in vec2 fragTex;
                        uniform bool useTexture;
                        uniform sampler2D diffuseMap;
                        void main() {
                            if (useTexture) {
                                alphaDiscard(texture(diffuseMap, fragTex).a);
                            }
                        })"
                    ) {
                /* Init shadow map */
                Framebuffer *depthFBO = Library::createFBO("shadowMap");
                depthFBO->attachDepthTexture(glm::ivec2(dimension), GL_LINEAR, GL_CLAMP_TO_BORDER);
                depthFBO->disableDraw();
                depthFBO->disableRead();
                glBindTexture(GL_TEXTURE_2D, depthFBO->mTextures[0]->mTextureID);
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, std::vector<float>{1.f, 1.f, 1.f, 1.f}.data());
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            virtual void render(const ECS& ecs) override {
                auto shadowCamera = ecs.getComponentTuple<ShadowCameraComponent, CameraComponent>();
                if (!shadowCamera) {
                    NEO_ASSERT(shadowCamera, "No shadow camera found");
                }
                auto camera = shadowCamera->get<CameraComponent>();

                auto fbo = Library::getFBO("shadowMap");
                auto & depthTexture = fbo->mTextures[0];

                fbo->bind();
                glClear(GL_DEPTH_BUFFER_BIT);
                glViewport(0, 0, depthTexture->mWidth, depthTexture->mHeight);

                bind();
                loadUniform("P", camera->getProj());
                loadUniform("V", camera->getView());

                for (auto& renderableIt : ecs.getComponentTuples<renderable::ShadowCasterRenderable, MeshComponent, SpatialComponent>()) {
                    auto renderable = renderableIt->get<renderable::ShadowCasterRenderable>();
                    auto renderableSpatial = renderableIt->get<SpatialComponent>();

                    loadUniform("M", renderableSpatial->getModelMatrix());

                    /* Bind texture */
                    loadTexture("diffuseMap", renderable->mAlphaMap);

                    /* DRAW */
                    renderableIt->get<MeshComponent>()->mMesh.draw();
                }


                unbind();
            }
        };

}