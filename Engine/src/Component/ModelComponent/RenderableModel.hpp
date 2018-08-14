#pragma once

#include "RenderableComponent.hpp"

#include "Model/Material.hpp"
#include "Model/Texture.hpp"

namespace neo {

    class MaterialRenderable : public RenderableComponent {

        public:
            MaterialRenderable(GameObject *go, Mesh *mesh, Material *mat) :
                RenderableComponent(go, mesh),
                material(mat)
            {}

            const Material & getMaterial() const { return *material; }

        protected:
            Material * material;
    };

    class TexturedRenderable : public RenderableComponent {

        public:

            TexturedRenderable(GameObject *go, Mesh *mesh, Texture *tex) :
                RenderableComponent(go, mesh),
                texture(tex)
            {}
                
            const Texture & getTexture() const { return *texture; }

         protected:
             Texture * texture;

    };

    // TODO : virtual inheritence 
    class MatTexturedRenderable : public RenderableComponent {

        public:

            MatTexturedRenderable(GameObject *go, Mesh *mesh, Material *mat, Texture *tex) :
                RenderableComponent(go, mesh),
                material(mat),
                texture(tex)
            {}

            const Material & getMaterial() const { return *material; }
            const Texture & getTexture() const { return *texture; }

        protected:
            Material * material;
            Texture * texture;


    };
}