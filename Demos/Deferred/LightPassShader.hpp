#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Engine/Engine.hpp"

using namespace neo;

namespace Deferred {
    class LightPassShader : public Shader {

    public:

        bool showLights = false;
        float showRadius = 0.1f;

        LightPassShader(const std::string& vert, const std::string& frag) :
            Shader("LightPassShader", vert, frag) {
            // Create render target
            auto lightFBO = Library::createFBO("lightpass");
            lightFBO->attachColorTexture({ 1, 1 }, TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color
            lightFBO->attachDepthTexture({ 1, 1 }, GL_NEAREST, GL_REPEAT); // depth

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage, &LightPassShader::_onFrameSizeChanged>(this);
        }

        virtual void render(const ECS& ecs) override {
            auto mainCamera = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
            if (!mainCamera) {
                return;
            }

            auto fbo = Library::getFBO("lightpass");
            fbo->bind();
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            bind();

            glBlendFunc(GL_ONE, GL_ONE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            loadUniform("showLights", showLights);
            loadUniform("showRadius", showRadius);

            auto&& [cameraEntity, _, camera, cameraSpatial] = *mainCamera;
            loadUniform("P", camera.getProj());
            loadUniform("V", mainCamera->get<CameraComponent>()->getView());
            loadUniform("invP", camera.getProj());
            loadUniform("invV", cameraSpatial.getView());
            loadUniform("camPos", cameraSpatial.getPosition());

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDiffuse", *gbuffer->mTextures[1]);
            loadTexture("gDepth", *gbuffer->mTextures[2]);

            /* Render light volumes */
            // TODO : instanced
            ecs.getView<LightComponent, SpatialComponent>().each([&](auto entity, auto light, auto spatial) {
                loadUniform("M", spatial.getModelMatrix());
                loadUniform("lightPos", spatial.getPosition());
                loadUniform("lightRadius", spatial.getScale().x);
                loadUniform("lightCol", light.mColor);

                // If camera is inside light 
                float dist = glm::distance(spatial.getPosition(), cameraSpatial.getPosition());
                if (dist - mainCamera->get<CameraComponent>()->getNearFar().x < spatial->getScale().x) {
                    glCullFace(GL_FRONT);
                }
                else {
                    glCullFace(GL_BACK);
                }

                Library::getMesh("sphere").mMesh->draw();
                });

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::Checkbox("Show lights", &showLights);
            if (showLights) {
                ImGui::SliderFloat("Show radius", &showRadius, 0.01f, 1.f);
            }
        }

        void LightPassShader::_onFrameSizeChanged(const FrameSizeMessage& msg) {
            Library::getFBO("lightpass")->resize(msg.mSize);
        }

    };
}
