#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

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
                uniform vec3 ambientColor;
                uniform vec3 diffuseColor;
                uniform vec3 specularColor;
                uniform float shine;
                uniform vec3 camPos;
                uniform vec3 lightPos;
                uniform vec3 lightCol;
                uniform vec3 lightAtt;
                out vec4 color;
                void main() {
                    vec4 albedo = texture(diffuseMap, fragTex);
                    albedo.rgb += diffuseColor;
                    alphaDiscard(albedo.a);
                    color.rgb = albedo.rgb * ambientColor + 
                                getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                    color.a = albedo.a;
                })")
        { }

        virtual void render() override {
            bind();

            /* Load PV */
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());

            /* Load light */
            if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
                loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
                loadUniform("lightCol", light->get<LightComponent>()->mColor);
                loadUniform("lightAtt", light->get<LightComponent>()->mAttenuation);
            }

            const auto& cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

            for (auto& renderableIt : Engine::getComponentTuples<renderable::PhongRenderable, MeshComponent, SpatialComponent>()) {
                auto renderable = renderableIt->get<renderable::PhongRenderable>();
                auto renderableSpatial = renderableIt->get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderableIt->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", renderableSpatial->getModelMatrix());
                loadUniform("N", renderableSpatial->getNormalMatrix());

                /* Bind texture */
                loadTexture("diffuseMap", renderable->mDiffuseMap);

                /* Bind material */
                Material& material = renderable->mMaterial;

                loadUniform("ambientColor", material.mAmbient);
                loadUniform("diffuseColor", material.mDiffuse);
                loadUniform("specularColor", material.mSpecular);
                loadUniform("shine", material.mShininess);

                /* DRAW */
                renderableIt->get<MeshComponent>()->mMesh.draw();
            }
        }
    };
}
