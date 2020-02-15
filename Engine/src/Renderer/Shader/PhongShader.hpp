#pragma once

#include "Engine.hpp"

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
                uniform bool useTexture;
                uniform vec3 ambient;
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

            // TODO : This can be cleaned up with a _render(Description {M, N, diffuse, ambient, ... });
            for (auto& renderable : Engine::getComponentTuples<renderable::PhongRenderable, ParentComponent>()) {
                auto parent = renderable->get<ParentComponent>();
                glm::mat4 pM(1.f);
                glm::mat3 pN(1.f);
                if (auto spatial = parent->getGameObject().getComponentByType<SpatialComponent>()) {
                    pM = spatial->getModelMatrix();
                    pN = spatial->getNormalMatrix();
                }

                for (auto& child : renderable->get<ParentComponent>()->childrenObjects) {
                    if (auto mesh = child->getComponentByType<MeshComponent>()) {
                        glm::mat4 M = pM;
                        glm::mat3 N = pN;
                        if (auto spatial = child->getComponentByType<SpatialComponent>()) {
                            M = M * spatial->getModelMatrix();
                            N = N * spatial->getNormalMatrix();
                        }
                        loadUniform("M", M);
                        loadUniform("N", N);

                        if (auto d = child->getComponentByType<DiffuseMapComponent>()) {
                            loadTexture("diffuseMap", d->mTexture);
                            loadUniform("useTexture", true);
                        }
                        else {
                            loadUniform("useTexture", false);
                        }

                        MaterialComponent* mc = parent->getGameObject().getComponentByType<MaterialComponent>();
                        if (auto m = child->getComponentByType<MaterialComponent>()) {
                            mc = m;
                        }
                        if (mc) {
                            loadUniform("ambient", mc->material.ambient);
                            loadUniform("diffuseColor", mc->material.diffuse);
                            loadUniform("specularColor", mc->material.specular);
                            loadUniform("shine", mc->material.shininess);
                        }

                        mesh->getMesh().draw();
                    }
                }
            }

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
                    loadTexture("diffuseMap", diffuseMap->mTexture);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* Bind material */
                if (auto matComp = renderable->mGameObject.getComponentByType<MaterialComponent>()) {
                    loadUniform("ambient", matComp->material.ambient);
                    loadUniform("diffuseColor", matComp->material.diffuse);
                    loadUniform("specularColor", matComp->material.specular);
                    loadUniform("shine", matComp->material.shininess);
                }

                /* DRAW */
                renderable->get<MeshComponent>()->getMesh().draw();
            }
        }
    };
}
