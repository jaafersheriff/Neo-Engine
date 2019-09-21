#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

using namespace neo;

class BlurShader : public Shader {

    public:

        float blurSteps = 100.f;
        float decay=0.965f;
        float density=0.756f;
        float weight=0.238;

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
            loadUniform("decay", decay);
            loadUniform("density", density);
            loadUniform("weight", weight);


            loadUniform("blurSteps", blurSteps);
            loadUniform("sunPos", Engine::getSingleComponent<SunComponent>()->getGameObject().getComponentByType<SpatialComponent>()->getPosition());
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mElementBufferID));
            mesh->draw();

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Steps", &blurSteps, 0.01f, 100.f);
            ImGui::SliderFloat("Decay", &decay, 0.01f, 1.f);
            ImGui::SliderFloat("Density", &density, 0.01f, 1.f);
            ImGui::SliderFloat("Weight", &weight, 0.01f, 1.f);
        }
};
