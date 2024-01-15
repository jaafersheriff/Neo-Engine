#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"

#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"

#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace neo {

    template<typename... CompTs>
    void drawPhong(const ECS& ecs, ECS::Entity cameraEntity, const Texture* shadowMap = nullptr, const SourceShader::ShaderDefines& inDefines = {}) {
        TRACY_GPU();
        const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
        auto&& [lightEntity, light, lightSpatial] = *ecs.getSingleView<LightComponent, SpatialComponent>();

        SourceShader::ShaderDefines parentDefines = inDefines;
        bool containsAlphaTest = false;
        if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
            containsAlphaTest = true;
            parentDefines.emplace("ALPHA_TEST");
            // Transparency sorting..for later
        //     glEnable(GL_BLEND);
        //     ecs.sort<AlphaTestComponent>([&cameraSpatial, &ecs](ECS::Entity entityLeft, ECS::Entity entityRight) {
        //         auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
        //         auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
        //         if (leftSpatial && rightSpatial) {
        //             return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) < glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
        //         }
        //         return false;
        //         });
        }

        glm::mat4 L;
        const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, OrthoCameraComponent, SpatialComponent>();
        const bool shadowsEnabled = shadowMap && shadowCamera.has_value();
        if (shadowsEnabled) {
            parentDefines.emplace("ENABLE_SHADOWS");
            const auto& [_, __, shadowOrtho, shadowCameraSpatial] = *shadowCamera;
            static glm::mat4 biasMatrix(
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f);
            L = biasMatrix * shadowOrtho.getProj() * shadowCameraSpatial.getView();
        }
        bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
        bool pointLight = ecs.has<PointLightComponent>(lightEntity);
        glm::vec3 attenuation(0.f);
        if (directionalLight) {
            parentDefines.emplace("DIRECTIONAL_LIGHT");
        }
        else if (pointLight) {
            attenuation = ecs.cGetComponent<PointLightComponent>(lightEntity)->mAttenuation;
            parentDefines.emplace("POINT_LIGHT");
        }
        else {
            NEO_FAIL("Phong light needs a directional or point light component");
        }

        const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
        const auto& view = ecs.getView<const PhongShaderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
        for (auto entity : view) {
            // VFC
            if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                if (!culled->isInView(ecs, entity, cameraEntity)) {
                    continue;
                }
            }

            if (containsAlphaTest) {
                NEO_ASSERT(!ecs.has<OpaqueComponent>(entity), "Entity has opaque and alpha test component?");
            }

            SourceShader::ShaderDefines drawDefines = parentDefines;
            const auto& material = view.get<const MaterialComponent>(entity);
            if (material.mDiffuseMap) {
                drawDefines.emplace("DIFFUSE_MAP");
            }
            if (material.mNormalMap) {
                drawDefines.emplace("NORMAL_MAP");
            }

            auto& resolvedShader = view.get<const PhongShaderComponent>(entity).getResolvedInstance(drawDefines);
            resolvedShader.bind();

            if (material.mDiffuseMap) {
                resolvedShader.bindTexture("diffuseMap", *material.mDiffuseMap);
            }
            else {
                resolvedShader.bindUniform("diffuseColor", material.mDiffuse);
            }

            if (material.mNormalMap) {
                resolvedShader.bindTexture("normalMap", *material.mNormalMap);
            }

            // UBO candidates
            {
                resolvedShader.bindUniform("P", P);
                resolvedShader.bindUniform("V", cameraSpatial->getView());
                resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
                resolvedShader.bindUniform("lightCol", light.mColor);
                if (directionalLight || shadowsEnabled) {
                    resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
                }
                if (pointLight) {
                    resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
                    resolvedShader.bindUniform("lightAtt", attenuation);
                }
                if (shadowsEnabled) {
                    resolvedShader.bindUniform("L", L);
                    resolvedShader.bindTexture("shadowMap", *shadowMap);
                }
            }

            const auto& drawSpatial = view.get<const SpatialComponent>(entity);
            resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
            resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());
            resolvedShader.bindUniform("ambientColor", material.mAmbient);
            resolvedShader.bindUniform("specularColor", material.mSpecular);
            resolvedShader.bindUniform("shine", material.mShininess);

            view.get<const MeshComponent>(entity).mMesh->draw();

            resolvedShader.unbind();
        }

        if (containsAlphaTest) {
            // glDisable(GL_BLEND);
        }
    }
}
