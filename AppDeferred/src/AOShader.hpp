#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class AOShader : public Shader {

    public:

        std::vector<glm::vec3> kernel;

        AOShader(const std::string &frag) :
            Shader("AO Shader", MasterRenderer::POST_PROCESS_VERT_FILE, frag) {
            // Create render target
            auto aoFBO = Loader::getFBO("AO");
            aoFBO->generate();
            aoFBO->attachColorTexture(Window::getFrameSize(), 1, GL_R16, GL_RED, GL_NEAREST, GL_REPEAT); // ao

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto aoFBO = Loader::getFBO("AO");
                aoFBO->textures[0]->width  = frameSize.x;
                aoFBO->textures[0]->height = frameSize.y;
                aoFBO->textures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, frameSize.x, frameSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));
            });

            // generate kernel
            generateKernel(64);
        }

        void generateKernel(unsigned size) {
            kernel.clear();
            for (unsigned i = 0; i < size; i++) {
                glm::vec3 sample = glm::normalize(glm::vec3(
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom( 0.f, 1.f)
                )) * Util::genRandom(0.f, 1.f);
                float scale = 1.f / (float)size;
                scale = Util::lerp(0.1f, 1.f, scale * scale);
                kernel.push_back(sample * scale);
            };
        }

        virtual void render(const CameraComponent &camera) override {
            CHECK_GL(glDisable(GL_DEPTH_TEST));

            // bind output
            Loader::getFBO("AO")->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
            glm::ivec2 frameSize = Window::getFrameSize();
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));
            bind();

            // bind gbuffer
            auto gbuffer = Loader::getFBO("gbuffer");
            gbuffer->textures[0]->bind();
            loadUniform("gNormal", gbuffer->textures[0]->textureId);

            // bind quad
            auto mesh = Loader::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->vaoId));

            // render
            mesh->draw();

            unbind();
            CHECK_GL(glEnable(GL_DEPTH_TEST));
        }
};