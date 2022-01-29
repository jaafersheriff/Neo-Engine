#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"

using namespace neo;

class GodRayOccluderShader : public Shader {

    public:

        GodRayOccluderShader(const std::string &vert, const std::string &frag) :
            Shader("GodRayOccluder Shader", vert, frag) {
        }

        virtual void render() override {
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            if (!camera) {
                return;
            }

            auto fbo = Library::getFBO("godray");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            /* Load PV */
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderable : Engine::getComponentTuples<SunOccluderComponent, MeshComponent, SpatialComponent>()) {
                auto renderableSpatial = renderable->get<SpatialComponent>();

                // VFC
                if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                    if (const auto& frustumPlanes = camera->mGameObject.getComponentByType<FrustumComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!frustumPlanes->isInFrustum(renderableSpatial->getPosition(), radius)) {
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
