#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/HardwareComponent/WindowDetailsComponent.hpp"

using namespace neo;

namespace GodRays {

    class GodRayOccluderShader : public Shader {

    public:

        GodRayOccluderShader(const std::string& vert, const std::string& frag) :
            Shader("GodRayOccluder Shader", vert, frag) {
        }

        virtual void render(const ECS& ecs) override {
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            if (!camera) {
                return;
            }

            auto fbo = Library::getFBO("godray");
            fbo->bind();
            auto windowDetails = ecs.getSingleComponent<WindowDetailsComponent>();
            NEO_ASSERT(windowDetails, "Window details don't exist");
            glm::ivec2 frameSize = windowDetails->mDetails.getSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            /* Load PV */
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            const auto cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

            for (auto& renderable : ecs.getComponentTuples<SunOccluderComponent, MeshComponent, SpatialComponent>()) {
                auto renderableSpatial = renderable->get<SpatialComponent>();

				// VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        if (!cameraFrustum->isInFrustum(*renderableSpatial, *boundingBox)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", renderableSpatial->getModelMatrix());

                /* Bind texture */
                loadTexture("alphaMap", renderable->get<SunOccluderComponent>()->mAlphaMap);

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }
    };
}
