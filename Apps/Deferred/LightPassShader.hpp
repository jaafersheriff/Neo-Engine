#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class LightPassShader : public Shader {

    public:

        bool showLights = false;
        float showRadius = 0.1f;

        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPassShader", vert, frag) {
            // Create render target
            auto lightFBO = Library::createFBO("lightpass");
            lightFBO->attachColorTexture({ 1, 1 }, TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).mFrameSize;
                Library::getFBO("lightpass")->resize(frameSize);
            });
        }

        virtual void render(const ECS& ecs) override {
            auto mainCamera = ecs.getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
            if (!mainCamera) {
                return;
            }

            auto fbo = Library::getFBO("lightpass");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();

            CHECK_GL(glBlendFunc(GL_ONE, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glEnable(GL_CULL_FACE));

            loadUniform("showLights", showLights);
            loadUniform("showRadius", showRadius);

            loadUniform("P", mainCamera->get<CameraComponent>()->getProj());
            loadUniform("V", mainCamera->get<CameraComponent>()->getView());
            loadUniform("invP", glm::inverse(mainCamera->get<CameraComponent>()->getProj()));
            loadUniform("invV", glm::inverse(mainCamera->get<CameraComponent>()->getView()));
            loadUniform("camPos", mainCamera->get<SpatialComponent>()->getPosition());

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal",  *gbuffer->mTextures[0]);
            loadTexture("gDiffuse", *gbuffer->mTextures[1]);
            loadTexture("gDepth",   *gbuffer->mTextures[2]);

            /* Render light volumes */
            for (auto& light : ecs.getComponentTuples<LightComponent, SpatialComponent>()) {
                auto spatial = light->get<SpatialComponent>();

                loadUniform("M", spatial->getModelMatrix());
                loadUniform("lightPos", spatial->getPosition());
                loadUniform("lightRadius", spatial->getScale().x);
                loadUniform("lightCol", light->get<LightComponent>()->mColor);

                // If mainCamera is inside light 
                float dist = glm::distance(spatial->getPosition(), mainCamera->get<SpatialComponent>()->getPosition());
                if (dist - mainCamera->get<CameraComponent>()->getNearFar().x < spatial->getScale().x) {
                    CHECK_GL(glCullFace(GL_FRONT));
                }
                else {
                    CHECK_GL(glCullFace(GL_BACK));
                }
                Library::getMesh("sphere")->draw();
            }

            unbind();
        }

        virtual void imguiEditor() {
            ImGui::Checkbox("Show lights", &showLights);
            if (showLights) {
                ImGui::SliderFloat("Show radius", &showRadius, 0.01f, 1.f);
            }
        }
};