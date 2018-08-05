// Acts as both library and loader 
#pragma once

#include <unordered_map>

#include "Model/Mesh.hpp"

namespace neo {

    class Texture;

    class Loader {

        public:
            static void init(const std::string &, bool);

            /* Library */
            static std::unordered_map<std::string, Mesh *> meshes;
            static std::unordered_map<std::string, Texture *> textures;

            /* Retrieve Mesh pointer from an .obj file*/
            static Mesh * getMesh(const std::string &, bool = false);
            /* Retrieve Texture pointer from an image file*/
            static Texture * getTexture(const std::string &, unsigned int);
            static Texture * getTexture(const std::string &, const std::string[6], unsigned int);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void resize(Mesh::MeshBuffers &);

            /* Private members */
            static std::string RES_DIR;
            static bool verbose;


    };
}