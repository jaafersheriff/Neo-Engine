#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "DemoInfra/IDemo.hpp"

#include "Messaging/Messenger.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class Engine;
    class ECS;
    class WindowSurface;
    class PostProcessShader;

    class Renderer {

        friend Engine;

        public:
            Renderer() = default;
            ~Renderer() = default;
            Renderer(const Renderer &) = delete;
            Renderer & operator=(const Renderer &) = delete;
            Renderer(Renderer &&) = delete;
            Renderer & operator=(Renderer &&) = delete;

            static std::string APP_SHADER_DIR;
            static std::string ENGINE_SHADER_DIR;
            static unsigned NEO_GL_MAJOR_VERSION;
            static unsigned NEO_GL_MINOR_VERSION;
            static std::string NEO_GLSL_VERSION;
            static glm::ivec3 NEO_MAX_COMPUTE_GROUP_SIZE;

            static void setDemoConfig(IDemo::Config);
            static void init();
            static void resetState();
            static void render(float, WindowSurface&, ECS&);
            static void clean();

            /* FBO */
            static void setDefaultFBO(const std::string &);

            /* Shaders */
            template <typename ShaderT, typename... Args> static ShaderT & addComputeShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPostProcessShader(Args &&...);
            template <typename ShaderT> static ShaderT& getShader();

        private:
            static void _imguiEditor(ECS& ecs);

            static Framebuffer* mDefaultFBO;
            static glm::vec3 mClearColor;

            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mComputeShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPreProcessShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mSceneShaders;
            static std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPostShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> _createShader(Args &&...);
            static std::vector<Shader *> _getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &);

            static void _renderPostProcess(Shader &, Framebuffer *, Framebuffer *, glm::ivec2, ECS&);
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
                NEO_ASSERT("Attempting to add a preprocess shader twice: %s", shader.second->mName.c_str());
                shader.second->mActive = true;
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
                NEO_ASSERT("Attempting to add a scene shader twice: %s", shader.second->mName.c_str());
                shader.second->mActive = true;
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
            auto ping = Library::createFBO("ping");
            ping->attachColorTexture({1, 1}, format);
            ping->attachDepthTexture({1, 1}, GL_NEAREST, GL_REPEAT);

            // Set default FBO if it's the back buffer
            if (mDefaultFBO == Library::getFBO("0")) {
                mDefaultFBO = ping;
            }

            auto pong = Library::createFBO("pong");
            pong->attachColorTexture({1, 1}, format);
            pong->mTextures.push_back(ping->mTextures[1]);

            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                Library::getFBO("ping")->resize(m.mFrameSize);
                Library::getFBO("pong")->resize(m.mFrameSize);
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

        for (auto& shader : mComputeShaders) {
            if (shader.first == typeI) {
                return reinterpret_cast<ShaderT &>(*shader.second);
            }
        }

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

        NEO_FAIL("Attempting to get a Shader that doesn't exist");
    }

}