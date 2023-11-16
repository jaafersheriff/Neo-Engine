#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

namespace neo {

	template<typename... CompTs>
    void drawShadows(Framebuffer& depthMap, const ECS& ecs) {
        TRACY_GPUN("drawShadows");

        depthMap.bind();
        depthMap.disableDraw();
        glBindTexture(GL_TEXTURE_2D, depthMap.mTextures[0]->mTextureID);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, std::vector<float>{1.f, 1.f, 1.f, 1.f}.data());
        glViewport(0, 0, depthMap.mTextures[0]->mWidth, depthMap.mTextures[0]->mHeight);

        bool containsAlphaTest = false;
        if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
            containsAlphaTest = true;
            glEnable(GL_BLEND);
        }

        auto shadowCameraView = ecs.getSingleView<ShadowCameraComponent, SpatialComponent>();
        if (!shadowCameraView) {
            NEO_ASSERT(shadowCameraView, "No shadow camera found");
        }
        auto&& [shadowCameraEntity, shadowCamera, shadowCameraSpatial] = *shadowCameraView;

        const auto& view = ecs.getView<const ShadowCasterShaderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
        for (auto entity : view) {
            // VFC
            if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                if (!culled->isInView(ecs, entity, shadowCameraEntity)) {
                    continue;
                }
            }

            SourceShader::ShaderDefines shaderDefines = {};
            auto material = ecs.cGetComponent<const MaterialComponent>(entity);
            Texture* alphaMap = nullptr;
            if (containsAlphaTest && material && material->mAlphaMap) {
                alphaMap = material->mAlphaMap;
                shaderDefines.emplace("ALPHA_TEST");
            }

            auto resolvedShader = view.get<const ShadowCasterShaderComponent>(entity).getResolvedInstance(shaderDefines);
            resolvedShader.bind();

            if (alphaMap) {
                resolvedShader.bindTexture("alphaMap", *alphaMap);
            }

            resolvedShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, OrthoCameraComponent>(shadowCameraEntity)->getProj());
            resolvedShader.bindUniform("V", shadowCameraSpatial.getView());
            resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
            view.get<const MeshComponent>(entity).mMesh->draw();

            resolvedShader.unbind();
        }

        if (containsAlphaTest) {
            glDisable(GL_BLEND);
        }
	}
}