// Acts as both library and loader 
#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include <string>
#include <vector>

#include "Renderer/GLObjects/Texture2D.hpp"
#include "Renderer/GLObjects/TextureCubeMap.hpp"

namespace neo {

    class Mesh;
    class Framebuffer;

    class Loader {

        public:
            static void init(const std::string &, bool);

            /* Load Mesh pointer from an .obj file */
            static Mesh* loadMesh(const std::string &, bool = false);

            /* Retrieve Texture pointer from an image file */
            static Texture2D* loadTexture(const std::string &, TextureFormat);
            static TextureCubeMap* loadTexture(const std::string &, const std::vector<std::string> &);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void _resize(Mesh*, std::vector<float>&, bool);

            /* Load a single texture file */
            static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, bool = true);
            static void _cleanTextureData(uint8_t*);

            /* Private members */
            static std::string RES_DIR;
            static bool mVerbose;


    };
}