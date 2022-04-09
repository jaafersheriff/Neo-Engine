#pragma once

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ECS/Messaging/Messenger.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Engine/Engine.hpp"

namespace neo {

    class SelectableShader : public Shader {

    public:

        SelectableShader() :
            Shader("Selectable Shader",
                R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        out vec2 fragTex;
                        void main() { gl_Position = P * V * M * vec4(vertPos, 1); })",
                R"(
                        uniform int componentID;
                        out int color;
                        void main() {
                            color = componentID;
                        })"
            ) {
            /* Init shadow map */
            Framebuffer* stencilBuffer = Library::createFBO("selectable");
            stencilBuffer->attachStencilTexture({1, 1}, GL_NEAREST, GL_CLAMP_TO_EDGE);
            stencilBuffer->disableDraw();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage>([this](const Message& message) {
                const FrameSizeMessage& msg(static_cast<const FrameSizeMessage&>(message));
                Library::getFBO("selectable")->resize(msg.mSize);
                });
        }

        // TODO : add hovered capability
        virtual void render(const ECS& ecs) override {
            const auto& mouseOpt = ecs.cGetComponent<MouseComponent>();
            NEO_ASSERT(mouseOpt, "wtf");
            if (!mouseOpt) {
                return;
            }
            auto&& [_, mouse] = *mouseOpt;
            if (!mouse.mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
                return;
            }

            auto fbo = Library::getFBO("selectable");
            fbo->bind();
            NEO_ASSERT(fbo->mTextures.size() > 0, "Selectable render target never initialized");

            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glClearStencil(0);
            glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight);

            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            bind();

            /* Load PV */
            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            if (cameraView) {
                auto&& [cameraEntity, __, cameraSpatial] = *cameraView;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", cameraSpatial.getView());
            }

            const auto cameraFrustum = ecs.cGetComponent<FrustumComponent>(std::get<0>(*cameraView));

            uint8_t rendered = 1;
            std::unordered_map<uint8_t, ECS::Entity> map;
            for (const auto&& [entity, selectable, mesh, spatial] : ecs.getView<SelectableComponent, MeshComponent, SpatialComponent>().each()) {
                ECS::Entity componentID = entity;

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(entity)) {
                        if (!cameraFrustum->isInFrustum(spatial, *boundingBox)) {
                            continue;
                        }
                    }
                }


                map.insert({ rendered, componentID });
                glStencilFunc(GL_ALWAYS, static_cast<GLuint>(rendered), 0);
                loadUniform("componentID", static_cast<int>(rendered));

                /* DRAW */
                loadUniform("M", spatial.getModelMatrix());
                mesh.mMesh->draw();
                rendered++;
                if (rendered > 256) {
                    break;
                }
            }

            uint8_t buffer[4];
            {
                MICROPROFILE_SCOPEI("Selectable Shader", "ReadPixels", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("Selectable Shader - ReadPixels", MP_AUTO);
                glm::ivec2 mousePos = glm::ivec2(mouse.mFrameMouse.getPos());
                glReadPixels(mousePos.x, mousePos.y, 1, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buffer);
            }
            uint8_t id = buffer[0];
            if (map[id] != mSelectedID) {

                mSelectedID = map[id];
                // Messenger::sendMessage<ComponentSelectedMessage>(nullptr, mSelectedID);
            }

            unbind();
        }

        private:
            ECS::Entity mSelectedID;
    };
}
