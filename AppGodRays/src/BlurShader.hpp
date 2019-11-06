#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

using namespace neo;

class BlurShader : public Shader {

    public:

        float mBlurSteps = 67.f;
        float mDecay=0.954f;
        float mDensity=0.78f;
        float mWeight=0.69f;
        float mContribution = 0.4f;

        BlurShader(const std::string &vert, const std::string &frag) :
            Shader("Blur Shader", vert, frag) {
            // Create blur 
            auto blur = Library::getFBO("godrayblur");
            blur->generate();

            // Format for color buffers
            TextureFormat format = { GL_R16, GL_RED, GL_NEAREST, GL_REPEAT };
            blur->attachColorTexture(Window::getFrameSize() / 2, 1, format); 
            blur->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("godrayblur")->resize(frameSize / 2);
            });

        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("godrayblur");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            Library::getFBO("godray")->mTextures[0]->bind();
            loadUniform("godray", Library::getFBO("godray")->mTextures[0]->mTextureID);
            loadUniform("decay", mDecay);
            loadUniform("density", mDensity);
            loadUniform("weight", mWeight);
            loadUniform("blurSteps", mBlurSteps);
            loadUniform("contribution", mContribution);

            // sun position in screen space
            if (const auto& sun = Engine::getSingleComponent<SunComponent>()) {
                glm::vec4 clipspace = camera.getProj() * camera.getView() * glm::vec4(sun->getGameObject().getComponentByType<SpatialComponent>()->getPosition(), 1.0);
                glm::vec3 ndcspace = glm::vec3(clipspace) / clipspace.w;
                glm::vec2 sspace = (glm::vec2(ndcspace) + 1.f) / 2.f;
                loadUniform("sunPos", sspace);
            }

            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            Library::getMesh("quad")->draw();

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Steps", &mBlurSteps, 0.01f, 100.f);
            ImGui::SliderFloat("Decay", &mDecay, 0.01f, 1.f);
            ImGui::SliderFloat("Density", &mDensity, 0.01f, 1.f);
            ImGui::SliderFloat("Weight", &mWeight, 0.01f, 1.f);
            ImGui::SliderFloat("Contribution", &mContribution, 0.01f, 1.f);
        }
};
