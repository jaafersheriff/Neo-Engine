#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/Framebuffer.hpp"

#include "Messaging/Messenger.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class Engine;
    class CameraComponent;
    class PostProcessShader;

    class Renderer {

        friend Engine;

        public:
            static std::string APP_SHADER_DIR;
            static unsigned NEO_GL_MAJOR_VERSION;
            static unsigned NEO_GL_MINOR_VERSION;
            static std::string NEO_GLSL_VERSION;

            static void init(const std::string &, CameraComponent *, glm::vec3 clearColor = glm::vec3(0.f));
            static void resetState();
            static void render(float);
            static void renderScene(const CameraComponent &);
            static void shutDown();

            static void setDefaultCamera(CameraComponent *cam) { mDefaultCamera = cam; }

            /* FBO */
            static void setDefaultFBO(const std::string &);

            /* Shaders */
            template <typename ShaderT, typename... Args> static ShaderT & addComputeShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPostProcessShader(Args &&...);
            template <typename ShaderT> static ShaderT& getShader();

        private:
            static CameraComponent* mDefaultCamera;
            static Framebuffer* mDefaultFBO;
            static glm::vec3 mClearColor;

            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mComputeShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPreProcessShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mSceneShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPostShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> _createShader(Args &&...);
            static std::vector<Shader *> _getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &);

            static void _renderPostProcess(Shader &, Framebuffer *, Framebuffer *);
    };

    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & Renderer::addComputeShader(Args &&... args) {
        std::type_index typeI(typeid(ShaderT));
        for (auto& shader : mComputeShaders) {
            if (shader.first == typeI) {
                return static_cast<ShaderT&>(*shader.second);
            }
        }
        mComputeShaders.emplace_back(typeI, _createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mComputeShaders.back().second);
    }

    template <typename ShaderT, typename... Args>
    ShaderT & Renderer::addPreProcessShader(Args &&... args) {
        std::type_index typeI(typeid(ShaderT));
        for (auto& shader : mPreProcessShaders) {
            if (shader.first == typeI) {
                return static_cast<ShaderT&>(*shader.second);
            }
        }
        mPreProcessShaders.emplace_back(typeI, _createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mPreProcessShaders.back().second);
    }

    template <typename ShaderT, typename... Args>
    ShaderT & Renderer::addSceneShader(Args &&... args) {
        std::type_index typeI(typeid(ShaderT));
        for (auto& shader : mSceneShaders) {
            if (shader.first == typeI) {
                return static_cast<ShaderT&>(*shader.second);
            }
        }
 
        mSceneShaders.emplace_back(typeI, _createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mSceneShaders.back().second);
    }

    template <typename ShaderT, typename... Args>
    ShaderT & Renderer::addPostProcessShader(Args &&... args) {
        static_assert(std::is_base_of<PostProcessShader, ShaderT>::value, "ShaderT must be derived from PostProcessShader");

        // Generate fbos if a post process shader exists
        if (!mPostShaders.size()) {
            TextureFormat format{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT };

            // Ping & pong 
            auto ping = Library::getFBO("ping");
            ping->generate();
            ping->attachColorTexture(Window::getFrameSize(), 4, format);
            ping->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);

            auto pong = Library::getFBO("pong");
            pong->generate();
            pong->attachColorTexture(Window::getFrameSize(), 4, format);
            pong->mTextures.push_back(ping->mTextures[1]);

            // Use ping as temporary fbo
            mDefaultFBO = ping;

            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                Library::getFBO("ping")->resize(m.frameSize);
                Library::getFBO("pong")->resize(m.frameSize);
            });
        }

        std::type_index typeI(typeid(ShaderT));
        mPostShaders.emplace_back(typeI, _createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*mPostShaders.back().second);
    }

    template <typename ShaderT, typename... Args>
    std::unique_ptr<ShaderT> Renderer::_createShader(Args &&... args) {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        return std::make_unique<ShaderT>(std::forward<Args>(args)...);
    }

    template <typename ShaderT> 
    ShaderT& Renderer::getShader(void) {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));

        for (auto& shader : mPreProcessShaders) {
            if (shader.first == typeI) {
                return reinterpret_cast<ShaderT &>(*shader.second);
            }
        }
        for (auto& shader : mSceneShaders) {
            if (shader.first == typeI) {
                return reinterpret_cast<ShaderT &>(*shader.second);
            }
        }
        for (auto& shader : mPostShaders) {
            if (shader.first == typeI) {
                return reinterpret_cast<ShaderT &>(*shader.second);
            }
        }


        assert(false);
    }

}