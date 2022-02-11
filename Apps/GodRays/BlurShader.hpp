#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine/Engine.hpp"

using namespace neo;

class BlurShader : public Shader {

    public:

        float mBlurSteps = 67.f;
        float mDecay=0.954f;
        float mDensity=0.78f;
        float mWeight=0.69f;
        float mContribution = 0.4f;

        BlurShader(const std::string &vert, const std::string& frag) :
            Shader("Blur Shader", vert, frag) {
            // Create blur 
            auto blur = Library::createFBO("godrayblur");
            TextureFormat format = { GL_R16, GL_RED, GL_NEAREST, GL_REPEAT };
            blur->attachColorTexture(WindowSurface::getFrameSize() / 2, format); 
            blur->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                NEO_UNUSED(m);
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("godrayblur")->resize(frameSize / 2);
            });

        }

        virtual void render() override {
            auto fbo = Library::getFBO("godrayblur");
            fbo->bind();
            glm::ivec2 frameSize = WindowSurface::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            loadTexture("godray", *Library::getFBO("godray")->mTextures[0]);

            loadUniform("decay", mDecay);
            loadUniform("density", mDensity);
            loadUniform("weight", mWeight);
            loadUniform("blurSteps", mBlurSteps);
            loadUniform("contribution", mContribution);

            auto mainCamera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(mainCamera, "No MainCamera exists");
            auto camera = mainCamera->get<CameraComponent>();

            // sun position in screen space
            if (const auto& sun = Engine::getSingleComponent<SunComponent>()) {
                glm::vec4 clipspace = camera->getProj() * camera->getView() * glm::vec4(sun->getGameObject().getComponentByType<SpatialComponent>()->getPosition(), 1.0);
                glm::vec3 ndcspace = glm::vec3(clipspace) / clipspace.w;
                glm::vec2 sspace = (glm::vec2(ndcspace) + 1.f) / 2.f;
                loadUniform("sunPos", sspace);
            }

            loadUniform("P", camera->getProj());
            loadUniform("V", camera->getView());

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
