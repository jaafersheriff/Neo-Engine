#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

#include <tracy/TracyOpenGL.hpp>

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
                                getPhong(fragNor, fragPos.rgb, camPos, lightPos - fragPos.xyz, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                    color.a = albedo.a;
                })")
        { }

        virtual void render(const ECS& ecs) override {
            TRACY_GPUN("PhongShader");
            bind();

            /* Load PV */
            const auto& camera = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            NEO_ASSERT(camera, "No main camera :(");
            auto&& [cameraEntity, _, cameraSpatial] = *camera;
            loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
            loadUniform("V", cameraSpatial.getView());
            loadUniform("camPos", cameraSpatial.getPosition());

            /* Load light */
            if (const auto& lightTuple = ecs.getSingleView<LightComponent, SpatialComponent>()) {
                auto&& [__, light, spatial] = *lightTuple;
                loadUniform("lightCol", light.mColor);
                loadUniform("lightAtt", light.mAttenuation);
                loadUniform("lightPos", spatial.getPosition());
            }

            for (const auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::PhongRenderable, MeshComponent, SpatialComponent>().each()) {

                // VFC
                if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                    if (!culled->isInView(ecs, entity, cameraEntity)) {
                        continue;
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
