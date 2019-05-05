#pragma once

#include <unordered_map>

namespace neo {

    class NeoEngine;
    class Mesh;
    class Texture;
    class Texture2D;
    class TextureCubeMap;
    class Framebuffer;

    class Library {

        friend NeoEngine;

        public:
            static Mesh* getMesh(const std::string&, bool = false);
            static Framebuffer* getFBO(const std::string&);

        private:
            static std::unordered_map<std::string, Mesh*> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;

            static void _insertMesh(const std::string&, Mesh*);

    };

}



