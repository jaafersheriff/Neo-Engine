#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/Framebuffer.hpp"

#include "Engine.hpp"

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
                Framebuffer *depthFBO = Library::getFBO("shadowMap");
                depthFBO->generate();
                depthFBO->attachDepthTexture(glm::ivec2(dimension), GL_LINEAR, GL_CLAMP_TO_BORDER);
                depthFBO->disableDraw();
                depthFBO->disableRead();
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, depthFBO->mTextures[0]->mTextureID));
                CHECK_GL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, std::vector<float>{1.f, 1.f, 1.f, 1.f}.data()));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
            }

            virtual void render(const CameraComponent &) override {
                auto shadowCamera = Engine::getSingleComponent<ShadowCameraComponent>();
                if (!shadowCamera) {
                    return;
                }
                auto camera = shadowCamera->getGameObject().getComponentByType<CameraComponent>();
                if (!camera) {
                    return;
                }

                auto fbo = Library::getFBO("shadowMap");
                auto & depthTexture = fbo->mTextures[0];

                fbo->bind();
                CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT));
                CHECK_GL(glViewport(0, 0, depthTexture->mWidth, depthTexture->mHeight));
                CHECK_GL(glCullFace(GL_FRONT));

                bind();
                loadUniform("P", camera->getProj());
                loadUniform("V", camera->getView());

                for (auto& renderable : Engine::getComponents<renderable::ShadowCasterRenderable>()) {
                    auto meshComponent = renderable->getGameObject().getComponentByType<MeshComponent>();
                    if (!meshComponent) {
                        continue;
                    }

                    /* Bind mesh */
                    const Mesh& mesh(meshComponent->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                    loadUniform("M", renderable->getGameObject().getSpatial()->getModelMatrix());

                    /* Bind texture */
                    auto texComp = renderable->getGameObject().getComponentByType<DiffuseMapComponent>();
                    if (texComp) {
                        auto texture = (const Texture2D *)(texComp->mTexture);
                        texture->bind();
                        loadUniform("diffuseMap", texture->mTextureID);
                        loadUniform("useTexture", true);
                    }
                    else {
                        loadUniform("useTexture", false);
                    }

                    /* DRAW */
                    mesh.draw();
                }


                unbind();
            }
        };

}