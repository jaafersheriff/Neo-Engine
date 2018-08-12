#pragma once

#include "RenderableComponent.hpp"

#include "Model/Material.hpp"
#include "Model/Texture.hpp"

namespace neo {

    class MaterialRenderable : public RenderableComponent {

        public:
            MaterialRenderable(GameObject &go, Mesh *mesh, Material *mat) :
                RenderableComponent(go, mesh),
                material(mat)
            {}

            const Material & getMaterial() const { return *material; }

        protected:
            Material * material;
    };

    class TexturedRenderable : public RenderableComponent {

        public:

            TexturedRenderable(GameObject &go, Mesh *mesh, Texture *tex) :
                RenderableComponent(go, mesh),
                texture(tex)
            {}
                
            const Texture & getTexture() const { return *texture; }

         protected:
             Texture * texture;

    };

    class MatTexturedRenderable : public MaterialRenderable, TexturedRenderable {

        public:

            MatTexturedRenderable(GameObject &go, Mesh *mesh, Material *mat, Texture *tex) :
                MaterialRenderable(go, mesh, mat),
                TexturedRenderable(go, mesh, tex)
            {}

    };
}