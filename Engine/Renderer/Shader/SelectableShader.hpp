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
            Shader("Selectable Shader",
                R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        out vec2 fragTex;
                        void main() { gl_Position = P * V * M * vec4(vertPos, 1); })",
                R"(
                        uniform uint componentID;
                        out uint color;
                        void main() {
                            color = componentID;
                        })"
            ) {
            /* Init shadow map */
            Framebuffer* fbo = Library::createFBO("selectable");
            fbo->attachColorTexture({ 1, 1 }, {
                GL_R32UI,
                GL_RED_INTEGER,
                GL_NEAREST,
                GL_CLAMP_TO_EDGE,
                GL_UNSIGNED_INT
                });
            fbo->attachDepthTexture({ 1, 1 }, GL_NEAREST, GL_CLAMP_TO_EDGE);
            fbo->initDrawBuffers();

            // Handle frame size changing
            // Messenger::addReceiver<FrameSizeMessage>(nullptr, [&](const Message& msg, ECS& ecs) {
            //     NEO_UNUSED(ecs);
            //     glm::uvec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
            //     Library::getFBO("selectable")->resize(frameSize);
            // });

        }

        virtual void render(const ECS& ecs) override {
            const auto& mouse = ecs.cGetComponent<MouseComponent>();
            NEO_ASSERT(mouse, "wtf");
            // TODO : add hovered capability
            if (!mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
                return;
            }

            auto fbo = Library::getFBO("selectable");
            fbo->bind();
            NEO_ASSERT(fbo->mTextures.size() > 0, "Selectable render target never initialized");

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight);

            bind();

            /* Load PV */
            auto camera = ecs.cGetComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera.get<CameraComponent>().getProj());
            loadUniform("V", camera.get<SpatialComponent>().getView());

            const auto cameraFrustum = ecs.cGetComponent<FrustumComponent>(camera.mEntity);

            uint8_t rendered = 1;
            std::unordered_map<uint8_t, ECS::Entity> map;
            for (const auto& tuple : ecs.getComponentTuples<SelectableComponent, MeshComponent, SpatialComponent>()) {
                const auto& [selectable, mesh, spatial] = tuple.get();
                ECS::Entity componentID = tuple.mEntity;

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(tuple.mEntity)) {
                        if (!cameraFrustum->isInFrustum(spatial, *boundingBox)) {
                            continue;
                        }
                    }
                }


                loadUniform("componentID", componentID);

                /* DRAW */
                loadUniform("M", spatial.getModelMatrix());
                mesh.mMesh->draw();
                rendered++;
                if (rendered > 256) {
                    break;
                }
            }

            uint32_t buffer;
            {
                MICROPROFILE_SCOPEI("Selectable Shader", "ReadPixels", MP_AUTO);
                MICROPROFILE_SCOPEGPUI("Selectable Shader - ReadPixels", MP_AUTO);
                glm::ivec2 mousePos = glm::ivec2(mouse->mFrameMouse.getPos());
                glReadnPixels(mousePos.x, mousePos.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, 4, &buffer);
            }

                mSelectedID = map[id];
                // Messenger::sendMessage<ComponentSelectedMessage>(nullptr, mSelectedID);
            }

            unbind();
        }

        private:
            ECS::Entity mSelectedID;
    };
}
