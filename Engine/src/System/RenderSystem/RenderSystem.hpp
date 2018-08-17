#pragma once

#include "System/System.hpp"
#include "Shader/Shader.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

namespace neo {

    class RenderableComponent;

    class RenderSystem : public System {

        public:
            RenderSystem(const std::string &dir, CameraComponent *cam) :
                System("Render System"),
                APP_SHADER_DIR(dir),
                defaultCamera(cam)
            {}

            virtual void init() override;
            virtual void update(float) override;
            void renderScene(const CameraComponent &);

            /* Shaders */
            std::vector<std::unique_ptr<Shader>> shaders;
            template <typename ShaderT, typename... Args> ShaderT & addShader(Args &&...);

            /* Map of Shader type to vector<RenderableComponent *> */
            mutable std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> renderables;
            template <typename ShaderT, typename CompT> const std::vector<CompT *> & getRenderables() const;
            void attachCompToShader(const std::type_index &, RenderableComponent *);
            void detachCompFromShader(const std::type_index &, RenderableComponent *);

        private:
            const std::string APP_SHADER_DIR;
            CameraComponent *defaultCamera;
    };

    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        shaders.emplace_back(std::make_unique<ShaderT>(APP_SHADER_DIR, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*shaders.back());
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