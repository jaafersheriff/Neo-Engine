#pragma once

#include <unordered_map>

namespace neo {

    class NeoEngine;
    class Mesh;
    class Texture;
    struct TextureFormat;
    class Framebuffer;

    class Library {

        friend NeoEngine;

        public:
            static Mesh* getMesh(const std::string&, bool = false);
            static Texture* getTexture(const std::string&, TextureFormat);
            static Framebuffer* getFBO(const std::string&);

        private:
            static std::unordered_map<std::string, Mesh*> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;

            static void _insertMesh(const std::string&, Mesh*);
            static void _insertTexture(const std::string&, Texture*);

    };

}



