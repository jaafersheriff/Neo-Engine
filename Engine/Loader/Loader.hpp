// Acts as both library and loader 
#pragma once

#include <unordered_map>

#include "Model/Mesh.hpp"

namespace neo {

    class Loader {

        public:
            static void init(const std::string &, bool);

            static std::unordered_map<std::string, Mesh *> meshes;

            /* Retrieve Mesh pointer from an .obj file*/
            static Mesh * getMesh(const std::string &);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void resize(Mesh::MeshBuffers &);

            /* GL Loaders */
            static void uploadMesh(const Mesh &);

            /* Private members */
            static std::string RES_DIR;
            static bool verbose;
    };
}