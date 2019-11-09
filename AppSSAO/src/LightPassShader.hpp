#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

using namespace neo;

class LightPassShader : public Shader {

    public:

        bool showLights = false;
        float showRadius = 0.1f;

        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPass Shader", vert, frag) {
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
            auto fbo = Library::getFBO("lightpass");
            fbo->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

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
            loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            gbuffer->mTextures[0]->bind();
            loadUniform("gNormal", gbuffer->mTextures[0]->mTextureID);
            gbuffer->mTextures[1]->bind();
            loadUniform("gDiffuse", gbuffer->mTextures[1]->mTextureID);
            gbuffer->mTextures[2]->bind();
            loadUniform("gDepth", gbuffer->mTextures[2]->mTextureID);

            /* Render light volumes */
            // TODO : instanced?
            for (auto & light : Engine::getComponents<LightComponent>()) {
                auto spat = light->getGameObject().getComponentByType<SpatialComponent>();
                loadUniform("M", spat->getModelMatrix());
                loadUniform("lightPos", spat->getPosition());
                loadUniform("lightRadius", spat->getScale().x);
                loadUniform("lightCol", light->mColor);

                // If camera is inside light 
                float dist = glm::distance(spat->getPosition(), camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());
                if (dist - camera.getNearFar().x < spat->getScale().x) {
                    CHECK_GL(glCullFace(GL_FRONT));
                }
                else {
                    CHECK_GL(glCullFace(GL_BACK));
                }
                Library::getMesh("ico_2", true)->draw();
            }

            unbind();
        }
        
        virtual void imguiEditor() override {
            ImGui::Checkbox("Show lights", &showLights);
            if (showLights) {
                ImGui::SameLine();
                ImGui::SliderFloat("Show radius", &showRadius, 0.01f, 1.f);
            }
        }
};
