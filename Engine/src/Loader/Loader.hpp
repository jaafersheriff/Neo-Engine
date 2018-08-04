// Acts as both library and loader 
#pragma once

#include <unordered_map>

#include "Model/Mesh.hpp"
#include "Model/ModelTexture.hpp"

namespace neo {

    class Loader {

        public:
            static void init(const std::string &, bool);

            /* Library */
            static std::unordered_map<std::string, Mesh *> meshes;
            static std::unordered_map<std::string, Texture *> textures;

            /* Static meshes */
            static const Mesh * getCube() { return cube; }
            static const Mesh * getQuad() { return quad; }

            /* Retrieve Mesh pointer from an .obj file*/
            static Mesh * getMesh(const std::string &, bool = false);
            /* Retrieve Texture pointer from an image file*/
            static Texture * getTexture(const std::string &, GLenum = GL_REPEAT);

            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void resize(Mesh::MeshBuffers &);

        private:
            /* Private members */
            static std::string RES_DIR;
            static bool verbose;

            /* Static meshes */
            static const Mesh * cube;
            static const Mesh * quad;
    };
}