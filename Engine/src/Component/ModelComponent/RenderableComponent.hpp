#pragma once

#include "Component/Component.hpp"

#include "GLHelper/Mesh.hpp"

#include <typeindex>

namespace neo {

    class Shader;

    class RenderableComponent : public Component {

        public:
            RenderableComponent(GameObject *go, Mesh *m) :
                Component(go),
                mMesh(m)
            {}

            virtual void init() override;
            virtual void kill() override;

            template <typename ShaderT> void addShaderType();
            template <typename ShaderT> void removeShaderType();
            void clearShaderTypes();

            virtual const Mesh & getMesh() const { return *mMesh; }
            void replaceMesh(Mesh *m) { this->mMesh = m; }
            const std::vector<std::type_index> & getShaders() { return mShaderTypes; }

        protected:
            Mesh * mMesh;
            std::vector<std::type_index> mShaderTypes;

        private:
            bool mIsInit = false;
    };

    /* Template implementation */
    template <typename ShaderT>
    void RenderableComponent::addShaderType() {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(mShaderTypes.begin(), mShaderTypes.end(), typeI);
        if (it == mShaderTypes.end()) {
            mShaderTypes.emplace_back(typeI);
            if (mIsInit) {
                MasterRenderer::attachCompToShader(typeI, this);
            }
        }
    }

    template <typename ShaderT>
    void RenderableComponent::removeShaderType() {
        static_assert(std::is_base_of<Shader, ShaderT>::value, "ShaderT must be a Shader type");
        static_assert(!std::is_same<ShaderT, Shader>::value, "ShaderT must be a derived Shader type");
        std::type_index typeI(typeid(ShaderT));
        auto it = std::find(mShaderTypes.begin(), mShaderTypes.end(), typeI);
        if (it != mShaderTypes.end()) {
            mShaderTypes.erase(it);
            if (mIsInit) {
                MasterRenderer::detachCompFromShader(typeI, this);
            }
        }
    }
}
