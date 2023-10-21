#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Util/Util.hpp"

#include <tracy/TracyOpenGL.hpp>

using namespace neo;

namespace FrustaFitting {
    class WireShader : public Shader {

    public:
        WireShader() :
            Shader("Wire Shader",
                R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        void main() {
                            gl_Position = P * V * M * vec4(vertPos, 1);
                        })",
                R"(
                        uniform vec3 wireColor;
                        out vec4 color;
                        void main() {
                            color = vec4(wireColor, 1.0);
                        })"
            )
        {}

        virtual void render(const ECS& ecs) override {
            ZoneScoped;
            TracyGpuZone("WireShader");
            bind();
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            /* Load PV */
            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            if (cameraView) {
                auto&& [cameraEntity, _, spatial] = *cameraView;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", spatial.getView());
            }

            auto mockSourceCamera = ecs.getSingleView<FrustumComponent, FrustumFitSourceComponent>(); // main mock camera 
            auto mockReceiverCamera = ecs.getSingleView<FrustumComponent, FrustumFitReceiverComponent>(); // shadow camera
            NEO_ASSERT(mockSourceCamera && mockReceiverCamera, "Dont use this without proper cameras set up");

            auto&& [mockSourceCameraEntity, _, __] = *mockSourceCamera;
            auto&& [mockReceiverCameraEntity, ___, ____] = *mockReceiverCamera;

            for(auto&& [e, bb, spatial] : ecs.getView<BoundingBoxComponent, SpatialComponent>().each()) {
                SpatialComponent sp = spatial;
                sp.setScale(spatial.getScale() * (bb.mMax - bb.mMin));

                bool mainView = false;
                bool shadowView = false;
                if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(e)) {
                    mainView = culled->isInView(ecs, e, mockSourceCameraEntity);
                    shadowView = culled->isInView(ecs, e, mockReceiverCameraEntity);
                }


                if (mainView || shadowView) {
                    glm::vec3 color(0.f);
                    if (mainView && shadowView) {
                        color = glm::vec3(0.7f);

                    }
                    else if (mainView) {
                        color = glm::vec3(1.0f);
                    }
                    loadUniform("wireColor", color);
                    loadUniform("M", sp.getModelMatrix());
                    Library::getMesh("cube").mMesh->draw();
                }
            }

            unbind();
        }
    };
}
