#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"
#include "Engine.hpp"

using namespace neo;

class LightPassShader : public Shader {

    public:

        bool showLights = false;
        float showRadius = 0.1f;

        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPassShader", vert, frag) {
            // Create render target
            auto lightFBO = Library::getFBO("lightpass");
            lightFBO->generate();
            lightFBO->attachColorTexture(Window::getFrameSize(), TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("lightpass")->resize(frameSize);
            });
        }

        virtual void render(const CameraComponent &camera) override {
            const auto camSpatial = camera.getGameObject().getComponentByType<SpatialComponent>();
            if (!camSpatial) {
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

            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("invP", glm::inverse(camera.getProj()));
            loadUniform("invV", glm::inverse(camera.getView()));
            loadUniform("camPos", camSpatial->getPosition());

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[1]);
            loadTexture("gDiffuse", *gbuffer->mTextures[2]);
            loadTexture("gDepth", *gbuffer->mTextures[3]);

            /* Render light volumes */
            for (auto& light : Engine::getComponentTuples<LightComponent, SpatialComponent>()) {
                auto spatial = light->get<SpatialComponent>();

                loadUniform("M", spatial->getModelMatrix());
                loadUniform("lightPos", spatial->getPosition());
                loadUniform("lightRadius", spatial->getScale().x);
                loadUniform("lightCol", light->get<LightComponent>()->mColor);

                // If camera is inside light 
                float dist = glm::distance(spatial->getPosition(), camSpatial->getPosition());
                if (dist - camera.getNearFar().x < spatial->getScale().x) {
                    CHECK_GL(glCullFace(GL_FRONT));
                }
                else {
                    CHECK_GL(glCullFace(GL_BACK));
                }
                Library::getMesh("sphere", true)->draw();
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