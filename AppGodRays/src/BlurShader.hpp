#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

using namespace neo;

class BlurShader : public Shader {

    public:

        float blurAmount = 1.f;

        BlurShader(const std::string &vert, const std::string &frag) :
            Shader("Blur Shader", vert, frag) {
            // Create blur 
            auto blur = Library::getFBO("godrayblur");
            blur->generate();

            // Format for color buffers
            TextureFormat format = { GL_R16, GL_RED, GL_NEAREST, GL_REPEAT };
            blur->attachColorTexture(Window::getFrameSize(), 1, format); 
            blur->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("godrayblur")->resize(frameSize);
            });

        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("godrayblur");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            Library::getFBO("godray")->mTextures[0]->bind();
            loadUniform("godray", Library::getFBO("godray")->mTextures[0]->mTextureID);
            loadUniform("blurAmount", blurAmount);

            auto mesh = Library::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->mVAOID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mElementBufferID));
            mesh->draw();

            unbind();
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat("Blur", &blurAmount, 0.f, 10.f);
        }
};
