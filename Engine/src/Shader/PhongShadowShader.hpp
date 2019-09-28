#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class PhongShadowShader : public Shader {

        public:

            PhongShadowShader(float b = 0.00005f, int pcfSize = 2) :
                Shader("PhongShadow Shader", 
                    R"(
                    layout(location = 0) in vec3 vertPos;
                    layout(location = 1) in vec3 vertNor;
                    layout(location = 2) in vec2 vertTex;
                    uniform mat4 P, V, M, L;
                    uniform mat3 N;
                    out vec4 fragPos;
                    out vec3 fragNor;
                    out vec2 fragTex;
                    out vec4 shadowCoord;
                    void main() {
                        fragPos = M * vec4(vertPos, 1.0);
                        fragNor = N * vertNor;
                        gl_Position = P * V * fragPos;
                        fragTex = vertTex;
                        shadowCoord = L * fragPos; 
                    })", 
                    R"(
                    #include "phong.glsl"
                    #include "shadowreceiver.glsl"
                    #include "alphaDiscard.glsl"

                    in vec4 fragPos;
                    in vec3 fragNor;
                    in vec2 fragTex;
                    in vec4 shadowCoord;
                    uniform float ambient;
                    uniform vec3 diffuseColor;
                    uniform vec3 specularColor;
                    uniform float shine;
                    uniform bool useTexture;
                    uniform sampler2D diffuseMap;
                    uniform vec3 camPos;
                    uniform vec3 lightPos, lightCol, lightAtt;
                    uniform sampler2D shadowMap;
                    uniform float bias;
                    uniform int pcfSize;
                    out vec4 color;
                    void main() {
                        vec4 albedo = vec4(diffuseColor, 1.f);
                        if (useTexture) {
                            albedo = texture(diffuseMap, fragTex);
                            alphaDiscard(albedo.a);
                        }

                        float visibility = getShadowVisibility(pcfSize, shadowMap, shadowCoord, bias);
                        vec3 phong = getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                        color.rgb = albedo.rgb * ambient + 
                                    visibility * phong;
                        color.a = albedo.a;
                    })"),
                bias(b),
                pcfSize(pcfSize)
            {}

            float bias;
            int pcfSize;
            const glm::mat4 biasMatrix = glm::mat4(
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f);

            virtual void render(const CameraComponent &camera) override {
                bind();

                /* Load Camera */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());
                if (const auto cameraSpatial = camera.getGameObject().getComponentByType<SpatialComponent>()) {
                    loadUniform("camPos", cameraSpatial->getPosition());
                }

                /* Load light */
                if (const auto shadowCamera = Engine::getSingleComponent<ShadowCameraComponent>()) {
                    if (const auto camera = shadowCamera->getGameObject().getComponentByType<CameraComponent>()) {
                        loadUniform("L", biasMatrix * camera->getProj() * camera->getView());
                    }
                }
                if (const auto light = Engine::getSingleComponent<LightComponent>()) {
                    if (const auto lightSpat = light->getGameObject().getComponentByType<SpatialComponent>()) {
                        loadUniform("lightPos", lightSpat->getPosition());
                        loadUniform("lightCol", light->mColor);
                        loadUniform("lightAtt", light->mAttenuation);
                    }
                }

                /* Bias */
                loadUniform("bias", bias);

                /* PCF */
                loadUniform("pcfSize", pcfSize);

                /* Bind shadow map */
                const Texture & texture(*Library::getFBO("shadowMap")->mTextures[0]);
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.mTextureID));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.mTextureID));
                loadUniform("shadowMap", texture.mTextureID);

                for (auto& renderable : Engine::getComponents<renderable::PhongShadowRenderable>()) {
                    auto meshComponent = renderable->getGameObject().getComponentByType<MeshComponent>();
                    auto renderableSpatial = renderable->getGameObject().getComponentByType<SpatialComponent>();
                    if (!meshComponent || !renderableSpatial) {
                        continue;
                    }

                    // VFC
                    if (const auto& boundingBox = renderable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                        if (const auto& frustumPlanes = camera.getGameObject().getComponentByType<FrustumComponent>()) {
                            float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                            if (!frustumPlanes->isInFrustum(renderableSpatial->getPosition(), radius)) {
                                continue;
                            }
                        }
                    }


                    /* Bind mesh */
                    const Mesh & mesh(meshComponent->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                    loadUniform("M", renderableSpatial->getModelMatrix());
                    loadUniform("N", renderableSpatial->getNormalMatrix());

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

                    /* Bind material */
                    if (auto material = renderable->getGameObject().getComponentByType<MaterialComponent>()) {
                        loadUniform("ambient", material->mAmbient);
                        loadUniform("diffuseColor", material->mDiffuse);
                        loadUniform("specularColor", material->mSpecular);
                        loadUniform("shine", material->mShine);
                    }

                    /* DRAW */
                    mesh.draw();
                }

                unbind();
            }

            virtual void imguiEditor() override {
                ImGui::SliderFloat("Bias", &bias, 1e-5, 1e-2);
                ImGui::SliderInt("PCF", &pcfSize, 0, 5);
            }
    };
}
