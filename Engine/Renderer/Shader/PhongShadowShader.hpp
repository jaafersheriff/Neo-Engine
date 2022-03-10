#pragma once

#include "ECS/ECS.hpp"

#include "Loader/Library.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongShadowRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

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
                    uniform vec3 ambientColor;
                    uniform vec3 diffuseColor;
                    uniform vec3 specularColor;
                    uniform float shine;
                    uniform sampler2D diffuseMap;
                    uniform vec3 camPos;
                    uniform vec3 lightPos, lightCol, lightAtt;
                    uniform sampler2D shadowMap;
                    uniform float bias;
                    uniform int pcfSize;
                    out vec4 color;
                    void main() {
                        vec4 albedo = texture(diffuseMap, fragTex);
                        alphaDiscard(albedo.a);
                        albedo.rgb += diffuseColor;

                        float visibility = getShadowVisibility(pcfSize, shadowMap, shadowCoord, bias);
                        vec3 phong = getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                        color.rgb = albedo.rgb * ambientColor + 
                                    max(visibility, 0.2) * phong;
                        color.a = albedo.a;
                    })"),
                bias(b),
                pcfSize(pcfSize)
            {}

            float bias = 0.002f;
            int pcfSize;
            const glm::mat4 biasMatrix = glm::mat4(
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f);

            virtual void render(const ECS& ecs) override {
                bind();

                /* Load PV */
                const auto& camera = ecs.cGetComponentTuple<MainCameraComponent, SpatialComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(camera.mEntity));
                loadUniform("V", camera.get<SpatialComponent>().getView());
                loadUniform("camPos", camera.get<SpatialComponent>().getPosition());

                /* Load light */
                if (const auto& shadowCamera = ecs.cGetComponentTuple<ShadowCameraComponent, SpatialComponent>()) {
                    auto _shadowCamera = ecs.cGetComponentAs<CameraComponent, OrthoCameraComponent>(shadowCamera.mEntity);
                    loadUniform("L", biasMatrix * _shadowCamera->getProj() * shadowCamera.get<SpatialComponent>().getView());
                }

                if (auto light = ecs.cGetComponentTuple<LightComponent, SpatialComponent>()) {
                    loadUniform("lightPos", light.get<SpatialComponent>().getPosition());
                    loadUniform("lightCol", light.get<LightComponent>().mColor);
                    loadUniform("lightAtt", light.get<LightComponent>().mAttenuation);
                }

                /* Bias */
                loadUniform("bias", bias);

                /* PCF */
                loadUniform("pcfSize", pcfSize);

                /* Bind shadow map */
                loadTexture("shadowMap", *Library::getFBO("shadowMap")->mTextures[0]);

                const auto& cameraFrustum = ecs.cGetComponent<FrustumComponent>(camera.mEntity);
                for (const auto& tuple : ecs.getComponentTuples<renderable::PhongShadowRenderable, MeshComponent, SpatialComponent>()) {
                    auto&& [renderable, mesh, spatial] = tuple.get();

                    // VFC
                    if (cameraFrustum) {
                        MICROPROFILE_SCOPEI("PhongShaderShader", "VFC", MP_AUTO);
                        if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(tuple.mEntity)) {
                            if (!cameraFrustum->isInFrustum(spatial, *boundingBox)) {
                                continue;
                            }
                        }
                    }

                    loadUniform("M", spatial.getModelMatrix());
                    loadUniform("N", spatial.getNormalMatrix());

                    /* Bind texture */
                    loadTexture("diffuseMap", *renderable.mDiffuseMap);

                    /* Bind material */
                    loadUniform("ambientColor", renderable.mMaterial.mAmbient);
                    loadUniform("diffuseColor", renderable.mMaterial.mDiffuse);
                    loadUniform("specularColor", renderable.mMaterial.mSpecular);
                    loadUniform("shine", renderable.mMaterial.mShininess);

                    /* DRAW */
                    mesh.mMesh->draw();
                }

                unbind();
            }

            virtual void imguiEditor() override {
                ImGui::SliderFloat("Bias", &bias, 1e-5f, 1e-2f);
                ImGui::SliderInt("PCF", &pcfSize, 0, 5);
            }
    };
}
