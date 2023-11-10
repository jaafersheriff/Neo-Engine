#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/NewRenderingComponents/PhongShaderComponent.hpp"
#include "ECS/Component/NewRenderingComponents/OpaqueComponent.hpp"

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderableComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"

#include "Renderer/GLObjects/NewShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace neo {

	template<typename... CompTs>
    void drawPhong(const ECS& ecs, ECS::Entity cameraEntity, const LightComponent& light, const SpatialComponent& lightSpatial, Texture* shadowMap = nullptr, const NewShader::ShaderDefines& inDefines = {}) {
        TRACY_GPU();
        const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);

        NewShader::ShaderDefines parentDefines = inDefines;
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
        auto lightCamera = ecs.cGetComponentAs<CameraComponent, OrthoCameraComponent>(lightEntity);
        bool shadowsEnabled = shadowMap && lightCamera;
        if (shadowsEnabled) {
            parentDefines.emplace("ENABLE_SHADOWS");
            L = lightCamera->getProj() * lightSpatial.getView();
        }

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

            NewShader::ShaderDefines drawDefines = parentDefines;
            const auto& material = view.get<const MaterialComponent>(entity);
            if (material.mDiffuseMap) {
                phongDefines.emplace("DIFFUSE_MAP");
            }
            if (material.mNormalMap) {
                phongDefines.emplace("NORMAL_MAP");
            }

            auto resolvedShader = view.get<const PhongShaderComponent>(entity).getResolvedInstance(phongDefines);
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
                resolvedShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                resolvedShader.bindUniform("V", cameraSpatial->getView());
                resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
                resolvedShader.bindUniform("lightCol", light.mColor);
                resolvedShader.bindUniform("lightAtt", light.mAttenuation);
                resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
                if (shadowsEnabled) {
                    loadUniform("lightDir", -lightSpatial.getLookDir());
                    loadUniform("L", L);
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
            //glDisable(GL_BLEND);
        }
	}
}
