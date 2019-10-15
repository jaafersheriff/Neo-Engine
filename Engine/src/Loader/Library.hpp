#pragma once

#include <unordered_map>

#include "GLObjects/Texture.hpp"

namespace neo {

    class Engine;
    class Mesh;
    class Framebuffer;

    class Library {

        friend Engine;

        public:
            static Mesh* getMesh(const std::string&, bool = false);
            static const std::unordered_map<std::string, Mesh*> getAllMeshes() { return mMeshes; }

            static Texture* getTexture(const std::string&, TextureFormat = TextureFormat{});

            static Texture* createEmptyTexture(const std::string&);
            static Texture* getCubemap(const std::string&, const std::vector<std::string> &);
            static Framebuffer* getFBO(const std::string&);

        private:
            // TODO 
            // template <typename T> static T* _find(const std::string&);
            static std::unordered_map<std::string, Mesh*> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;

            static void _insertMesh(const std::string&, Mesh*);
            static void _insertTexture(const std::string&, Texture*);


    };

}



