#pragma once

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "DemoInfra/IDemo.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

#define RENDERER_MP_ENTERD(define, group, name) \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(name.size()), name.c_str()); \
    MICROPROFILE_DEFINE(define, group, name.c_str(), MP_AUTO);\
    MICROPROFILE_ENTER(define);\
    MICROPROFILE_DEFINE_GPU(define, name.c_str(),  MP_AUTO);\
    MICROPROFILE_GPU_ENTER(define)

#define RENDERER_MP_ENTER(name) \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name); \
    MICROPROFILE_ENTERI("Renderer", name, MP_AUTO);\
    MICROPROFILE_GPU_ENTERI("RendererGPU", name, MP_AUTO)

#define RENDERER_MP_LEAVE() \
    if (glIsEnabled(GL_DEBUG_OUTPUT)) glPopDebugGroup(); \
    MICROPROFILE_LEAVE();\
    MICROPROFILE_GPU_LEAVE()


namespace neo {

    class Engine;
    class ECS;
    class ImGuiManager;
    class WindowSurface;
    class PostProcessShader;
    class BlitShader;
    struct FrameSizeMessage;

    class Renderer {

        friend ImGuiManager;

        struct FrameStats {
            uint32_t mNumDraws = 0;
            uint32_t mNumTriangles = 0;
            uint32_t mNumShaders = 0;
            uint32_t mNumUniforms = 0;
            uint32_t mNumSamplers = 0;
        };

        struct RendererDetails {
            int mGLMajorVersion = 0;
            int mGLMinorVersion = 0;
            std::string mGLSLVersion = "";
            glm::ivec3 mMaxComputeWorkGroupCount = { 0,0,0 };
            glm::ivec3 mMaxComputeWorkGroupSize = { 0,0,0 };
            int mMaxComputeWorkInvocations = 0;
            std::string mVendor = "";
            std::string mRenderer = "";
            std::string mShadingLanguage = "";
        };

        public:
            Renderer(int GLMajor, int GLMinor);
            ~Renderer() = default;
            Renderer(const Renderer &) = delete;
            Renderer & operator=(const Renderer &) = delete;
            Renderer(Renderer &&) = delete;
            Renderer & operator=(Renderer &&) = delete;

            std::string APP_SHADER_DIR = "";
            std::string ENGINE_SHADER_DIR = "../Engine/shaders/";
            FrameStats mStats = {};
            RendererDetails mDetails = {};

            void setDemoConfig(IDemo::Config);
            void init();
            void resetState();
            void render(WindowSurface& window, const ECS& ecs);
            void clean();

            /* Shaders */
            template <typename ShaderT, typename... Args> ShaderT & addComputeShader(Args &&...);
            template <typename ShaderT, typename... Args> ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> ShaderT & addPostProcessShader(Args &&...);
            template <typename ShaderT> ShaderT& getShader();

            void imGuiEditor(WindowSurface& window);
        private:
            Framebuffer* mBackBuffer;
			Framebuffer* mDefaultFBO;
            glm::vec3 mClearColor;
            BlitShader* mBlitShader = nullptr;
            bool mShowBB = false;

            std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mComputeShaders;
            std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPreProcessShaders;
            std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mSceneShaders;
            std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> mPostShaders;
            template <typename ShaderT, typename... Args> std::unique_ptr<ShaderT> _createShader(Args &&...);
            std::vector<Shader *> _getActiveShaders(std::vector<std::pair<std::type_index, std::unique_ptr<Shader>>> &);

            void _renderPostProcess(Shader &, Framebuffer *, Framebuffer *, glm::ivec2, const ECS& ecs);
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

            Messenger::addReceiver<FrameSizeMessage>(nullptr, [&](const Message &msg) {
                const FrameSizeMessage & m(static_cast<const FrameSizeMessage &>(msg));
                auto ping = Library::getFBO("ping");
                if (ping) {
                    ping->resize(m.mSize);
                }
                auto pong = Library::getFBO("pong");
                if (pong) {
                    pong->resize(m.mSize);
                }
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