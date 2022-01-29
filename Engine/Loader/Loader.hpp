// Acts as both library and loader 
#pragma once

#include "GL/glew.h"

#include <string>
#include <vector>

#include "Renderer/GLObjects/Texture2D.hpp"
#include "Renderer/GLObjects/TextureCubeMap.hpp"

// Remove?
#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Material.hpp"

namespace neo {

    struct Asset {
        Mesh* mesh;
        Material material;
        Texture* ambient_tex = nullptr;            // map_Ka
        Texture* diffuse_tex = nullptr;            // map_Kd
        Texture* specular_tex = nullptr;           // map_Ks
        Texture* displacement_tex = nullptr;       // disp
        // Texture* alpha_tex = nullptr;              // map_d
        // Texture* bump_tex = nullptr;               // map_bump, bump
        // Texture* specular_highlight_tex; // map_Ns
    };


    class Loader {

        public:
            static void init(const std::string &, bool);

            /* Load Mesh pointer from an .obj file */
            static Mesh* loadMesh(const std::string &, bool = false);
            static std::vector<Asset> loadMultiAsset(const std::string &);

            /* Retrieve Texture pointer from an image file */
            static Texture2D* loadTexture(const std::string &, TextureFormat);
            static TextureCubeMap* loadTexture(const std::string &, const std::vector<std::string> &);

        private:
            /* Resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void _resize(Mesh*, std::vector<float>&, bool);

            /* Load a single texture file */
            static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, TextureFormat, bool = true);
            static void _cleanTextureData(uint8_t*);

            /* Private members */
            static std::string RES_DIR;
            static bool mVerbose;


    };
}