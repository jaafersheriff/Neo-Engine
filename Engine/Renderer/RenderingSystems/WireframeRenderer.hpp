#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/WireframeShaderComponent.hpp"

namespace neo {

	template<typename... CompTs>
    void drawWireframe(const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
        TRACY_GPU();

        // TODO - stash the old state?
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const auto& view = ecs.getView<const WireframeShaderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
        for (auto entity : view) {
            // VFC
            if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
                if (!culled->isInView(ecs, entity, cameraEntity)) {
                    continue;
                }
            }

            auto resolvedShader = view.get<const WireframeShaderComponent>(entity).getResolvedInstance(inDefines);
            resolvedShader.bind();

            resolvedShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
            resolvedShader.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());
            resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
            resolvedShader.bindUniform("color", view.get<const WireframeShaderComponent>(entity).mColor);

            view.get<const MeshComponent>(entity).mMesh->draw();
        }

        // TODO - stash the old state
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}