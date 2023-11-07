// Acts as both library and loader 
#pragma once

#include "GL/glew.h"

#include "ECS/Component/RenderableComponent/MaterialComponent.hpp"

#include "Renderer/GLObjects/Texture2D.hpp"
#include "Renderer/GLObjects/TextureCubeMap.hpp"

// Remove?
#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Material.hpp"

namespace neo {

    class Engine;

   struct MeshData {
        Mesh* mMesh;
        glm::vec3 mMin{ 0.f, 0.f, 0.f };
        glm::vec3 mMax{ 0.f, 0.f, 0.f };

        glm::vec3 mBasePosition{ 0.f, 0.f, 0.f };
        glm::vec3 mBaseScale{ 1.f, 1.f, 1.f };
    };

    struct Asset {
        MeshData meshData;
        MaterialComponent material;
        // Texture* ambient_tex = nullptr;            // map_Ka
        // Texture* bump_tex = nullptr;               // map_bump, bump
        // Texture* specular_highlight_tex; // map_Ns
    };

    class Loader {
        friend Engine;

        public:
            Loader() = default;
            ~Loader() = default;
            Loader(const Loader&) = delete;
            Loader& operator=(const Loader&) = delete;

            static void init(const std::string &resDir, const std::string &shaderDir);

            static const char* loadFileString(const std::string&);

            /* Load Mesh pointer from an .obj file */
            static MeshData loadMesh(const std::string &, bool = false);
            static std::vector<Asset> loadMultiAsset(const std::string &);

            /* Retrieve Texture pointer from an image file */
            static Texture2D* loadTexture(const std::string &, TextureFormat);
            static TextureCubeMap* loadTexture(const std::string &, const std::vector<std::string> &);

        private:
            /* Optionally resize mesh vertex buffers so all the vertices are [-1, 1] */
            static void _findMetaData(MeshData& meshData, std::vector<float>& verts, bool doResize);

            /* Load a single texture file */
            static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, TextureFormat, bool = true);
            static void _cleanTextureData(uint8_t*);

            /* Private members */
            static std::string APP_RES_DIR;
            static std::string APP_SHADER_DIR;
            static std::string ENGINE_RES_DIR;
            static std::string ENGINE_SHADER_DIR;
    };
}