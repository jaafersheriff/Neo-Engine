#pragma once

#include "System/System.hpp"
#include "Shader/Shader.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"

#include <vector>
#include <memory>

namespace neo {

    class RenderSystem : public System {

        public:
            RenderSystem(const std::string &dir, const CameraComponent *camera) :
                APP_SHADER_DIR(dir),
                camera(camera)
            {}

            virtual void init() override;
            virtual void update(float) override;

            std::vector<std::unique_ptr<Shader>> shaders;
            template <typename ShaderT, typename... Args> ShaderT & addShader(Args &&...);

        private:
            const std::string APP_SHADER_DIR;
            const CameraComponent *camera;
    };


    /* Template implementation */
    template <typename ShaderT, typename... Args>
    ShaderT & RenderSystem::addShader(Args &&... args) {
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        static_assert(!std::is_same<Shader, ShaderT>::value, "ShaderT must be a derived Shader type");
        shaders.emplace_back(std::make_unique<ShaderT>(APP_SHADER_DIR, std::forward<Args>(args)...));
        return static_cast<ShaderT &>(*shaders.back());
    }
}