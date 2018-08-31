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

    class MasterRenderer {

        public:
            static std::string APP_SHADER_DIR;

            static void init(const std::string &, CameraComponent *);
            static void render(float);
            static void renderScene(const CameraComponent &);

            static void setDefaultCamera(CameraComponent *cam) { defaultCamera = cam; }

            /* FBO */
            static Framebuffer * getFBO(const std::string &);
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

            static std::unordered_map<std::string, std::unique_ptr<Framebuffer>> framebuffers;
            static Framebuffer *defaultFBO;
            static Framebuffer * findFBO(const std::string &name);

            static std::vector<std::unique_ptr<Shader>> preShaders;
            static std::vector<std::unique_ptr<Shader>> sceneShaders;
            static std::vector<std::unique_ptr<Shader>> postShaders;
            template <typename ShaderT, typename... Args> static std::unique_ptr<ShaderT> createShader(Args &&...);

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
        postShaders.emplace_back(createShader<ShaderT>(std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*postShaders.back());
    }
    template <typename ShaderT, typename... Args>
    std::unique_ptr<ShaderT> MasterRenderer::createShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
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