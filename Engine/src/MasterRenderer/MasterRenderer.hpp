#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/Framebuffer.hpp"

#include "Messaging/Messenger.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class NeoEngine;
    class CameraComponent;
    class PostProcessShader;

    class MasterRenderer {

        friend NeoEngine;

        public:
            static std::string APP_SHADER_DIR;

            static void init(const std::string &, CameraComponent *, glm::vec3 clearColor = glm::vec3(0.f));
            static void resetState();
            static void render(float);
            static void renderScene(const CameraComponent &);

            static void setDefaultCamera(CameraComponent *cam) { mDefaultCamera = cam; }

            /* FBO */
            static void setDefaultFBO(const std::string &);

            /* Shaders */
            template <typename ShaderT, typename... Args> static ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPostProcessShader(Args &&...);

        private:
            static CameraComponent* mDefaultCamera;
            static Framebuffer* mDefaultFBO;
            static glm::vec3 mClearColor;

            static std::vector<std::unique_ptr<Shader>> mPreProcessShaders;
            static std::vector<std::unique_ptr<Shader>> mSceneShaders;
            static std::vector<std::unique_ptr<Shader>> mPostShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> _createShader(Args &&...);
            static std::vector<Shader *> _getActiveShaders(std::vector<std::unique_ptr<Shader>> &);

            static void _renderPostProcess(Shader &, Framebuffer *, Framebuffer *);
    };

    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addPreProcessShader(Args &&... args) {
        mPreProcessShaders.emplace_back(_createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mPreProcessShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addSceneShader(Args &&... args) {
        mSceneShaders.emplace_back(_createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mSceneShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addPostProcessShader(Args &&... args) {
        static_assert(std::is_base_of<PostProcessShader, ShaderT>::value, "ShaderT must be derived from PostProcessShader");

        // Generate fbos if a post process shader exists
        if (!mPostShaders.size()) {
            // Ping & pong 
            auto ping = Loader::getFBO("ping");
            ping->generate();
            ping->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT);
            ping->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
            auto pong = Loader::getFBO("pong");
            pong->generate();
            pong->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT);
            pong->mTextures.push_back(ping->mTextures[1]);

            // Use ping as temporary fbo
            mDefaultFBO = ping;

            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                Loader::getFBO("ping")->resize(m.frameSize);
                Loader::getFBO("pong")->resize(m.frameSize);
            });
        }
        mPostShaders.emplace_back(_createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mPostShaders.back());
    }
    template <typename ShaderT, typename... Args>
    std::unique_ptr<ShaderT> MasterRenderer::_createShader(Args &&... args) {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        return std::make_unique<ShaderT>(std::forward<Args>(args)...);
    }

}