#pragma once

#include "System/System.hpp"

#include <vector>
#include <memory>

namespace neo {

    class Shader;

    class RenderSystem : public System {

        public:
            virtual void init() override;
            virtual void update(float) override;

            std::vector<std::unique_ptr<Shader>> shaders;
            template <typename ShaderT, typename... Args> ShaderT & addShader(Args &&...);
    };


    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        shaders.emplace_back(std::make_unique<ShaderT>(NeoEngine::APP_RES_DIR, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*shaders.back());
    }
}