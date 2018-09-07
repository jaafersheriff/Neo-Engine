// Acts as both library and loader 
#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLHelper/Mesh.hpp"

#include <unordered_map>

namespace neo {

    class Texture;
    class Texture2D;
    class TextureCubeMap;
    class Framebuffer;
    class Material;

    class Loader {

        public:
            static void init(const std::string &, bool);

            /* Library */
            static std::unordered_map<std::string, Mesh *> meshes;
            static std::unordered_map<std::string, Texture *> textures;
            static std::unordered_map<std::string, Framebuffer *> framebuffers;
            static std::unordered_map<std::string, Material *> materials;

            /* Retrieve Mesh pointer from an .obj file */
            static Mesh * getMesh(const std::string &, bool = false);

            /* Retrieve Texture pointer from an image file */
            static Texture2D * getTexture(const std::string &, GLint = GL_RGBA, GLenum = GL_RGBA, GLint = GL_LINEAR, GLenum = GL_REPEAT);
            static TextureCubeMap * getTexture(const std::string &, const std::vector<std::string> &);

            /* Retrieve FBO */
            static Framebuffer * getFBO(const std::string &);

            /* Retrieve material */
            static Material * getMaterial(const std::string &, float = 0.2f, glm::vec3 = glm::vec3(0.f), glm::vec3 = glm::vec3(1.f), float = 20.f);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void resize(Mesh::MeshBuffers &);

            /* Find in library */
            static Texture * findTexture(const std::string &name);

            /* Private members */
            static std::string RES_DIR;
            static bool verbose;


    };
}