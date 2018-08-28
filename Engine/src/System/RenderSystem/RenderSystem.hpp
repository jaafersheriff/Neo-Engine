#pragma once

#include "System/System.hpp"
#include "Shader/Shader.hpp"
#include "Framebuffer.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class RenderableComponent;
    class CameraComponent;

    class RenderSystem : public System {

        public:
            RenderSystem(const std::string &dir, CameraComponent *cam) :
                System("Render System"),
                APP_SHADER_DIR(dir),
                defaultCamera(cam)
            {}

            const std::string APP_SHADER_DIR;

            virtual void init() override;
            virtual void update(float) override;
            void renderScene(const CameraComponent &) const;

            void setDefaultCamera(CameraComponent *cam) { defaultCamera = cam; }

            /* FBO */
            std::unordered_map<std::string, std::unique_ptr<Framebuffer>> framebuffers;
            Framebuffer * createFBO(const std::string &);

            /* Shaders */
            std::vector<std::unique_ptr<Shader>> preShaders;
            std::vector<std::unique_ptr<Shader>> sceneShaders;
            std::vector<std::unique_ptr<Shader>> postShaders;
            template <typename ShaderT, typename... Args> ShaderT & addPreProcessShader(Args &&...);
            template <typename ShaderT, typename... Args> ShaderT & addSceneShader(Args &&...);
            template <typename ShaderT, typename... Args> ShaderT & addPostProcessShader(Args &&...);

            /* Map of Shader type to vector<RenderableComponent *> */
            mutable std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> renderables;
            template <typename ShaderT, typename CompT> const std::vector<CompT *> & getRenderables() const;
            void attachCompToShader(const std::type_index &, RenderableComponent *);
            void detachCompFromShader(const std::type_index &, RenderableComponent *);

        private:
            CameraComponent *defaultCamera;
            Framebuffer *defaultFBO;
    };

    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addPreProcessShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        preShaders.emplace_back(std::make_unique<ShaderT>(*this, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*preShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addSceneShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        sceneShaders.emplace_back(std::make_unique<ShaderT>(*this, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*sceneShaders.back());
    }
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addPostProcessShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        postShaders.emplace_back(std::make_unique<ShaderT>(*this, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*postShaders.back());
    }

    template <typename ShaderT, typename CompT> 
    const std::vector<CompT *> & RenderSystem::getRenderables() const {
        std::type_index typeI(typeid(ShaderT));
        auto it(renderables.find(typeI));
        if (it == renderables.end()) {
            renderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = renderables.find(typeI);
        }
        return reinterpret_cast<const std::vector<CompT *> &>(*(it->second));
    }

}