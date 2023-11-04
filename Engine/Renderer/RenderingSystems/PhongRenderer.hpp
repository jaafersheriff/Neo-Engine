#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderableComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "Renderer/GLObjects/NewShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace neo {

	template<typename... CompTs>
    void drawPhong(const ECS& ecs, ECS::Entity cameraEntity, const LightComponent& light, const SpatialComponent& lightSpatial, const NewShader::ShaderDefines& inDefines = {}) {
        const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);

        // bool containsAlphaTest = contains(AlphaTestComponent, CompTs...);
        // if (containsAlphaTest) {
        //     registry.sort<PhongShaderComponent, SpatialComponent, CompTs...>([cameraSpatial](const auto entityLeft, const auto entityRight)) {
        //         return glm::distance(cameraSpatial.getPosition(), registry.getComponent<SpatialComponent>(entityLeft).getPosition()) < glm::distance(cameraSpatial.getPosition(), registry.getComponent<SpatialComponent>(entityRight).getPosition());
        //     }
        // }

        const auto& view = ecs.getView<const PhongShaderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
        for (auto entity : view) {
            // VFC
            if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                if (!culled->isInView(ecs, entity, cameraEntity)) {
                    continue;
                }
            }
 
            NewShader::ShaderDefines phongDefines = inDefines;
            // if (containsAlphaTest) {
            //     phongDefines.emplace("ALPHA_TEST");
            // }

            const auto& material = view.get<const MaterialComponent>(entity);
            if (material.mDiffuseMap) {
                phongDefines.emplace("DIFFUSE_MAP");
            }
            auto resolvedShader = view.get<const PhongShaderComponent>(entity).getResolvedInstance(phongDefines);
            resolvedShader.bind();

            if (material.mDiffuseMap) {
                resolvedShader.bindTexture("diffuseMap", *material.mDiffuseMap);
            }
            else {
                resolvedShader.bindUniform("diffuseColor", material.mDiffuse);
            }

            resolvedShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
            resolvedShader.bindUniform("V", cameraSpatial->getView());
            resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
            resolvedShader.bindUniform("lightCol", light.mColor);
            resolvedShader.bindUniform("lightAtt", light.mAttenuation);
            resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
            const auto& drawSpatial = view.get<const SpatialComponent>(entity);
            resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
            resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());
            resolvedShader.bindUniform("ambientColor", material.mAmbient);
            resolvedShader.bindUniform("specularColor", material.mSpecular);
            resolvedShader.bindUniform("shine", material.mShininess);
        
            view.get<const MeshComponent>(entity).mMesh->draw();
        }

	}
}