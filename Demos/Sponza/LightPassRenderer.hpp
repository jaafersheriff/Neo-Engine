#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Library.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"

using namespace neo;

namespace Sponza {
    void drawPointLights(const ECS& ecs, Framebuffer& gbuffer, ECS::Entity cameraEntity, const glm::uvec2 resolution, const float debugRadius) {
        TRACY_GPU();

        auto* lightResolveShader = Library::createSourceShader("PointLightResolveShader", SourceShader::ConstructionArgs{
            { ShaderStage::VERTEX, "sponza/pointlightresolve.vert"},
            { ShaderStage::FRAGMENT, "sponza/pointlightresolve.frag" }
        });

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        ShaderDefines defines;
        MakeDefine(SHOW_LIGHTS);
        if (debugRadius > 0.f) {
            defines.set(SHOW_LIGHTS);
        }
        auto& resolvedShader = lightResolveShader->getResolvedInstance(defines);
        resolvedShader.bind();

        resolvedShader.bindUniform("debugRadius", debugRadius);
        resolvedShader.bindUniform("resolution", glm::vec2(resolution));

        const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
        const auto& cameraSpatial = ecs.cGetComponent<const SpatialComponent>(cameraEntity);
        resolvedShader.bindUniform("P", P);
        resolvedShader.bindUniform("V", cameraSpatial->getView());
        resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

        /* Bind gbuffer */
        resolvedShader.bindTexture("gAlbedo", *gbuffer.mTextures[0]);
        resolvedShader.bindTexture("gSpecular", *gbuffer.mTextures[1]);
        resolvedShader.bindTexture("gWorld", *gbuffer.mTextures[2]);
        resolvedShader.bindTexture("gNormal", *gbuffer.mTextures[3]);

        /* Render light volumes */
        // TODO : instanced
        const auto& view = ecs.getView<const LightComponent, const PointLightComponent, const SpatialComponent>();
        for (auto entity : view) {
            // TODO : Could do VFC
            const auto spatial = ecs.cGetComponent<const SpatialComponent>(entity);
            resolvedShader.bindUniform("M", spatial->getModelMatrix());
            resolvedShader.bindUniform("lightPos", spatial->getPosition());
            resolvedShader.bindUniform("lightRadius", spatial->getScale().x);
            resolvedShader.bindUniform("lightCol", ecs.cGetComponent<const LightComponent>(entity)->mColor);

            // If camera is inside light 
            float dist = glm::distance(spatial->getPosition(), cameraSpatial->getPosition());
            if (dist - ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getNearFar().x < spatial->getScale().x) {
                glCullFace(GL_FRONT);
            }
            else {
                glCullFace(GL_BACK);
            }

            Library::getMesh("sphere").mMesh->draw();
        }

        // TODO - reset state
    }

    void drawDirectionalLights(const ECS& ecs, ECS::Entity cameraEntity, Framebuffer& gbuffer, Texture* shadowMap = nullptr) {
        TRACY_GPU();

        ShaderDefines defines;

        glm::mat4 L;
        const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, OrthoCameraComponent, SpatialComponent>();
        const bool shadowsEnabled = shadowMap && shadowCamera.has_value();
        MakeDefine(ENABLE_SHADOWS);
        if (shadowsEnabled) {
            defines.set(ENABLE_SHADOWS);
            const auto& [_, __, shadowOrtho, shadowCameraSpatial] = *shadowCamera;
            static glm::mat4 biasMatrix(
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f);
            L = biasMatrix * shadowOrtho.getProj() * shadowCameraSpatial.getView();
        }

        auto* lightResolveShader = Library::createSourceShader("DirectionalLightResolveShader", SourceShader::ConstructionArgs{
            { ShaderStage::VERTEX, "quad.vert"},
            { ShaderStage::FRAGMENT, "sponza/directionallightresolve.frag" }
        });
        auto& resolvedShader = lightResolveShader->getResolvedInstance(defines);
        resolvedShader.bind();

        if (shadowsEnabled) {
            resolvedShader.bindUniform("lightTransform", L);
            resolvedShader.bindTexture("shadowMap", *shadowMap);
        }

        auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
        resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
        resolvedShader.bindUniform("lightCol", light.mColor);

        const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
        resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());

        /* Bind gbuffer */
        resolvedShader.bindTexture("gAlbedo", *gbuffer.mTextures[0]);
        resolvedShader.bindTexture("gSpecular", *gbuffer.mTextures[1]);
        resolvedShader.bindTexture("gWorld", *gbuffer.mTextures[2]);
        resolvedShader.bindTexture("gNormal", *gbuffer.mTextures[3]);

        glDisable(GL_DEPTH_TEST);

        Library::getMesh("quad").mMesh->draw();

        // TODO - reset GL state
    }
}
