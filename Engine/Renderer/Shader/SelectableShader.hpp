#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

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
            Shader("Selectable",
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
            Messenger::addReceiver<FrameSizeMessage>(nullptr, [&](const Message& msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::uvec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
                Library::getFBO("selectable")->resize(frameSize);
            });

        }

        virtual void render(const ECS& ecs) override {
            auto mouse = ecs.getSingleComponent<MouseComponent>();
            // TODO : add hovered capability
            if (!mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
                return;
            }

            auto fbo = Library::getFBO("selectable");
            fbo->bind();
            NEO_ASSERT(fbo->mTextures.size() > 0, "Selectable render target never initialized");

            CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
            CHECK_GL(glClearStencil(0));
            CHECK_GL(glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));

            CHECK_GL(glEnable(GL_STENCIL_TEST));
            CHECK_GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            bind();

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            const auto& cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

            uint8_t rendered = 1;
            std::unordered_map<uint8_t, uint32_t> map;
            for (const auto& renderable : ecs.getComponentTuples<SelectableComponent, MeshComponent, SpatialComponent>()) {
                auto renderableSpatial = renderable->get<SpatialComponent>();
                uint32_t componentID = renderable->get<SelectableComponent>()->mID;

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        if (!cameraFrustum->isInFrustum(*renderableSpatial, *boundingBox)) {
                            continue;
                        }
                    }
                }


                map.insert({ rendered, componentID });
                CHECK_GL(glStencilFunc(GL_ALWAYS, static_cast<GLuint>(rendered), 0));
                loadUniform("componentID", static_cast<int>(rendered));

                /* DRAW */
                loadUniform("M", renderableSpatial->getModelMatrix());
                renderable->get<MeshComponent>()->mMesh.draw();
                rendered++;
            }

            uint8_t buffer[4];
            {
                MICROPROFILE_SCOPEI("Selectable Shader", "ReadPixels", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("Selectable Shader - ReadPixels", MP_AUTO);
                glm::ivec2 mousePos = glm::ivec2(mouse->mFrameMouse.getPos());
                CHECK_GL(glReadPixels(mousePos.x, mousePos.y, 1, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buffer));
            }
            uint8_t id = buffer[0];
            if (map[id] != mSelectedID) {
                mSelectedID = map[id];
                Messenger::sendMessage<ComponentSelectedMessage>(nullptr, mSelectedID);
            }

            unbind();
        }

        private:
            uint32_t mSelectedID = 0;
    };
}
