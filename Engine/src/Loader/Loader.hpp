// Acts as both library and loader 
#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "Model/Mesh.hpp"

#include <unordered_map>

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
            static Texture * getTexture(const std::string &, GLint inFormat = GL_RGBA, GLenum format = GL_RGBA, GLint filter = GL_LINEAR, GLenum mode = GL_REPEAT);
            static Texture * getTexture(const std::string, const std::vector<std::string> &);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void resize(Mesh::MeshBuffers &);

            /* Private members */
            static std::string RES_DIR;
            static bool verbose;


    };
}