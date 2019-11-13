#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class PhongShader : public Shader {

    public:

        PhongShader() :
            Shader("Phong Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 1) in vec3 vertNor;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                uniform mat3 N;
                out vec4 fragPos;
                out vec3 fragNor;
                out vec2 fragTex;
                void main() {
                    fragPos = M * vec4(vertPos, 1.0);
                    fragNor = N * vertNor;
                    fragTex = vertTex;
                    gl_Position = P * V * fragPos;
                })",
                R"(
                #include "phong.glsl"
                #include "alphaDiscard.glsl"

                in vec4 fragPos;
                in vec3 fragNor;
                in vec2 fragTex;
                uniform sampler2D diffuseMap;
                uniform bool useTexture;
                uniform float ambient;
                uniform vec3 diffuseColor;
                uniform vec3 specularColor;
                uniform float shine;
                uniform vec3 camPos;
                uniform vec3 lightPos;
                uniform vec3 lightCol;
                uniform vec3 lightAtt;
                out vec4 color;
                void main() {
                    vec4 albedo = vec4(diffuseColor, 1.f);
                    if (useTexture) {
                        albedo = texture(diffuseMap, fragTex);
                        alphaDiscard(albedo.a);
                    }
                    color.rgb = albedo.rgb * ambient + 
                                getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                    color.a = albedo.a;
                })")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            if (const auto cameraSpatial = camera.getGameObject().getComponentByType<SpatialComponent>()) {
                loadUniform("camPos", cameraSpatial->getPosition());
            }

            /* Load light */
            if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
                loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
                loadUniform("lightCol", light->get<LightComponent>()->mColor);
                loadUniform("lightAtt", light->get<LightComponent>()->mAttenuation);
            }

            const auto& cameraFrustum = camera.getGameObject().getComponentByType<FrustumComponent>();

            for (auto& renderable : Engine::getComponentTuples<renderable::PhongRenderable, MeshComponent, SpatialComponent>()) {
                auto renderableSpatial = renderable->get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", renderableSpatial->getModelMatrix());
                loadUniform("N", renderableSpatial->getNormalMatrix());

                /* Bind texture */
                if (auto diffuseMap = renderable->mGameObject.getComponentByType<DiffuseMapComponent>()) {
                    diffuseMap->mTexture.bind();
                    loadUniform("diffuseMap", diffuseMap->mTexture.mTextureID);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* Bind material */
                if (auto matComp = renderable->mGameObject.getComponentByType<MaterialComponent>()) {
                    loadUniform("ambient", matComp->mAmbient);
                    loadUniform("diffuseColor", matComp->mDiffuse);
                    loadUniform("specularColor", matComp->mSpecular);
                    loadUniform("shine", matComp->mShine);
                }

                /* DRAW */
                renderable->get<MeshComponent>()->getMesh().draw();
            }

            unbind();
        }
    };
}
