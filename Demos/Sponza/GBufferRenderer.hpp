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

Framebuffer& createGBuffer(glm::uvec2 targetSize) {
    return *Library::getPooledFramebuffer({ targetSize, {
        // Albedo
        TextureFormat{
            TextureTarget::Texture2D,
            GL_RGB16,
            GL_RGB,
        },
        // Normals
        TextureFormat{
            TextureTarget::Texture2D,
            GL_RGB16,
            GL_RGB,
        },
        // Depth
        TextureFormat{
            TextureTarget::Texture2D,
            GL_DEPTH_COMPONENT16,
            GL_DEPTH_COMPONENT,
        }
    } }, "Gbuffer");
}

template<typename... CompTs>
void drawGBuffer(const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
    TRACY_GPU();

    ShaderDefines passDefines(inDefines);
    bool containsAlphaTest = false;
    MakeDefine(ALPHA_TEST);
    if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
        containsAlphaTest = true;
        passDefines.set(ALPHA_TEST);
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

    const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
    const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);

    ShaderDefines drawDefines(passDefines);
    const auto& view = ecs.getView<const GBufferShaderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
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

        drawDefines.reset();

        const auto& material = view.get<const MaterialComponent>(entity);
        MakeDefine(DIFFUSE_MAP);
        MakeDefine(NORMAL_MAP);
        if (material.mDiffuseMap) {
            drawDefines.set(DIFFUSE_MAP);
        }
        if (material.mNormalMap) {
            drawDefines.set(NORMAL_MAP);
        }

        auto& resolvedShader = view.get<const GBufferShaderComponent>(entity).getResolvedInstance(drawDefines);
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
        }

        const auto& drawSpatial = view.get<const SpatialComponent>(entity);
        resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
        resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

        view.get<const MeshComponent>(entity).mMesh->draw();
    }

    if (containsAlphaTest) {
        // glDisable(GL_BLEND);
    }
}