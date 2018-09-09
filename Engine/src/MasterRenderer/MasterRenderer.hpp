#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/Framebuffer.hpp"

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
            static const char * POST_PROCESS_VERT_FILE;

            static void init(const std::string &, CameraComponent *);
            static void resetState();
            static void render(float);
            static void renderScene(const CameraComponent &);

            static void setDefaultCamera(CameraComponent *cam) { defaultCamera = cam; }

            /* FBO */
            static void setDefaultFBO(const std::string &);

            /* Shaders */
            template <typename ShaderT, typename... Args> static ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> static ShaderT & addPostProcessShader(Args &&...);

            /* Map of Shader type to vector<RenderableComponent *> */
            template <typename ShaderT, typename CompT> static const std::vector<CompT *> & getRenderables();
            static void attachCompToShader(const std::type_index &, RenderableComponent *);
            static void detachCompFromShader(const std::type_index &, RenderableComponent *);

        private:
            static CameraComponent *defaultCamera;
            static Framebuffer * defaultFBO;

            static std::vector<std::unique_ptr<Shader>> preShaders;
            static std::vector<std::unique_ptr<Shader>> sceneShaders;
            static std::vector<std::unique_ptr<Shader>> postShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> createShader(Args &&...);
            static std::vector<Shader *> getActiveShaders(std::vector<std::unique_ptr<Shader>> &);

            static void renderPostProcess(Shader &, Framebuffer *, Framebuffer *);

            static std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> renderables;
    };

    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addPreProcessShader(Args &&... args) {
        preShaders.emplace_back(createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*preShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addSceneShader(Args &&... args) {
        sceneShaders.emplace_back(createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*sceneShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & MasterRenderer::addPostProcessShader(Args &&... args) {
        // Generate fbos if a post process shader exists
        if (!postShaders.size()) {
            // New default FBO so we're not rendering to 0
            defaultFBO = Loader::getFBO("1");
            defaultFBO->generate();
            defaultFBO->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT);
            defaultFBO->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
 
            // Ping & pong 
            auto ping = Loader::getFBO("ping");
            ping->generate();
            ping->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT);
            ping->textures.push_back(defaultFBO->textures[1]);
            auto pong = Loader::getFBO("pong");
            pong->generate();
            pong->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT);
            pong->textures.push_back(defaultFBO->textures[1]);

            // TODO 
            // Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
            //     const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
            //     CHECK_GL(glViewport(0, 0, m.frameSize.x, m.frameSize.y));
            // });
        }
        postShaders.emplace_back(createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*postShaders.back());
    }
    template <typename ShaderT, typename... Args>
    std::unique_ptr<ShaderT> MasterRenderer::createShader(Args &&... args) {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        return std::make_unique<ShaderT>(std::forward<Args>(args)...);
    }

    template <typename ShaderT, typename CompT> 
    const std::vector<CompT *> & MasterRenderer::getRenderables() {
        std::type_index typeI(typeid(ShaderT));
        auto it(renderables.find(typeI));
        if (it == renderables.end()) {
            renderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = renderables.find(typeI);
        }
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }

}