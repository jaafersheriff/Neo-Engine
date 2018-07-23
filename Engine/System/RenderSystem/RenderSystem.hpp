#pragma once

#include "System/System.hpp"
#include "Shader/Shader.hpp"

#include "Component/RenderableComponent/RenderableComponent.hpp"
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

            virtual std::string name() { return "Render System"; }
            virtual void init() override;
            virtual void update(float) override;

            const CameraComponent & getCamera() const { return *camera; }

            /* Shaders */
            std::vector<std::unique_ptr<Shader>> shaders;
            template <typename ShaderT, typename... Args> ShaderT & addShader(Args &&...);

            /* Map of Shader type to vector<RenderableComponent *> */
            mutable std::unordered_map<std::type_index, std::unique_ptr<std::vector<RenderableComponent *>>> renderables;
            template <typename ShaderT, typename CompT> const std::vector<CompT *> & getRenderables() const;
            template <typename ShaderT> void attachShaderToComp(RenderableComponent *);
            template <typename ShaderT> void removeShaderToComp(RenderableComponent *);

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

    template <typename ShaderT>
    void RenderSystem::attachShaderToComp(RenderableComponent *renderable) {
        /* Get vector<RenderableComponent *> that matches this shader */
        std::type_index typeI(typeid(ShaderT));
        auto it(renderables.find(typeI));
        if (it == renderables.end()) {
            renderables.emplace(typeI, std::make_unique<std::vector<RenderableComponent *>>());
            it = renderables.find(typeI);
        }

        /* There should only be a one-to-one pairing of shaders and RenderComponent ref */
        if (renderable->addShaderType(typeI)) {
            it->second->emplace_back(renderable);
        }
    }

    template <typename ShaderT> 
    void RenderSystem::removeShaderToComp(RenderableComponent *renderable) {
        std::type_index typeI(typeid(ShaderT));
        assert(renderables.count(typeI));
        /* Look in active components in reverse order */
        if (renderables.count(typeI)) {
        auto & comps(*renderables.at(typeI));
            for (int i = int(comps.size()) - 1; i >= 0; --i) {
                if (comps[i] == renderable) {
                    comps.erase(comps.begin() + i);
                    break;
                }
            }
        }
        renderable->removeShaderType(typeI);
    }

}