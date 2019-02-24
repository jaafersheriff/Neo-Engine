#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/Framebuffer.hpp"

#include "Messaging/Messenger.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class RenderableComponent;
    class CameraComponent;
    class NeoEngine;

    class MasterRenderer {

        friend NeoEngine;

        public:
            static std::string APP_SHADER_DIR;
            static const char* POST_PROCESS_VERT_FILE;

            static void init(const std::string &, CameraComponent *);
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

            /* Map of Shader type to vector<RenderableComponent *> */
            template <typename ShaderT, typename CompT> static const std::vector<CompT *>& getRenderables();
            static void attachCompToShader(const std::type_index &, RenderableComponent *);
            static void detachCompFromShader(const std::type_index &, RenderableComponent *);

        private:
            static CameraComponent* mDefaultCamera;
            static Framebuffer* mDefaultFBO;

            static std::vector<std::unique_ptr<Shader>> mPreProcessShaders;
            static std::vector<std::unique_ptr<Shader>> mSceneShaders;
            static std::vector<std::unique_ptr<Shader>> mPostShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> _createShader(Args &&...);
            static std::vector<Shader *> _getActiveShaders(std::vector<std::unique_ptr<Shader>> &);

            static void _renderPostProcess(Shader &, Framebuffer *, Framebuffer *);

            static std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> mRenderables;
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

    template <typename ShaderT, typename CompT> 
    const std::vector<CompT *> & MasterRenderer::getRenderables() {
        std::type_index typeI(typeid(ShaderT));
        auto it(mRenderables.find(typeI));
        if (it == mRenderables.end()) {
            mRenderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = mRenderables.find(typeI);
        }
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }

}