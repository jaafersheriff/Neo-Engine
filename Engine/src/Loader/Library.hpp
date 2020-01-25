#pragma once

#include <unordered_map>

#include "GLObjects/Texture.hpp"

#include "Util/Util.hpp"

namespace neo {

    class Engine;
    class Mesh;
    class Framebuffer;

    class Library {

        friend Engine;

        public:
            static Mesh* getMesh(const std::string&, bool = false);
            static Mesh* createEmptyMesh(const std::string&);
            static const std::unordered_map<std::string, Mesh*> getAllMeshes() { return mMeshes; }

            static Texture* getTexture(const std::string&, TextureFormat = TextureFormat{});

            template <typename T>
            static Texture* createEmptyTexture(const std::string&, TextureFormat, glm::uvec3 = glm::uvec3(1));
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

    template <typename T>
    Texture* Library::createEmptyTexture(const std::string& name, TextureFormat format, glm::uvec3 size) {
        static_assert(std::is_base_of<Texture, T>::value, "T must be a Texture type");
        static_assert(!std::is_same<T, Texture>::value, "T must be a derived Texture type");

        auto it = mTextures.find(name);
        NEO_ASSERT(it == mTextures.end(), "Texture already found");
        void* data;
        switch(format.type) {
        case GL_UNSIGNED_BYTE:
            uint8_t d = 0xFF;
            data = &d;
            break;
        case GL_FLOAT:
            float d = 255;
            data = &d;
            break;
        case GL_INT:
            int d = 1.f;
            data = &d;
            break;
        default:
            NEO_ASSERT(false, "Trying to create an empty texture with an unsupported type");
            break;
        }
        Texture* t = new T(format, size, data);
        _insertTexture(name, t);
        return t;
    }


}



