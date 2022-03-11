#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

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

        virtual void render(const ECS& ecs) override {
            bind();

            /* Load PV */
            const auto& camera = ecs.getView<MainCameraComponent, SpatialComponent>();
            NEO_ASSERT(camera.size_hint() == 1, "No main camera exists");

            loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(camera.front())->getProj());
            loadUniform("V", camera.get<const SpatialComponent>(camera.front()).getView());
            loadUniform("camPos", camera.get<const SpatialComponent>(camera.front()).getPosition());

            /* Load light */
            const auto& light = ecs.getView<LightComponent, SpatialComponent>();
            NEO_ASSERT(light.size_hint() == 1, "");
            loadUniform("lightPos", light.get<const SpatialComponent>(light.front()).getPosition());
            loadUniform("lightCol", light.get<const LightComponent>(light.front()).mColor);
            loadUniform("lightAtt", light.get<const LightComponent>(light.front()).mAttenuation);

            const auto& cameraFrustum = ecs.cGetComponent<FrustumComponent>(camera.front());

            for (const auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::PhongRenderable, MeshComponent, SpatialComponent>().each()) {

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(entity)) {
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
                const Material& material = renderable.mMaterial;

                loadUniform("ambientColor", material.mAmbient);
                loadUniform("diffuseColor", material.mDiffuse);
                loadUniform("specularColor", material.mSpecular);
                loadUniform("shine", material.mShininess);

                /* DRAW */
                mesh.mMesh->draw();
            }
        }
    };
}
