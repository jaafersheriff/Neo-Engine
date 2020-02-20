#pragma once

#include <unordered_map>

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Util/Util.hpp"

namespace neo {

    class Engine;
    class Framebuffer;

    class Library {

        friend Engine;

        public:
            static Mesh* createEmptyMesh(const std::string&);
            static Mesh* getMesh(const std::string&);
            static Mesh* loadMesh(const std::string&, bool = false);
            static void loadMesh(const std::string&, Mesh* mesh);
            static const std::unordered_map<std::string, Mesh*> getAllMeshes() { return mMeshes; }

            template <typename T>
            static Texture* createEmptyTexture(const std::string&, TextureFormat, glm::uvec3 = glm::uvec3(1));
            static Texture* getTexture(const std::string&);
            static Texture* loadTexture(const std::string&, TextureFormat = TextureFormat{});
            static Texture* loadCubemap(const std::string&, const std::vector<std::string> &);


            static Framebuffer* createFBO(const std::string&);
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
        if (format.mType == GL_UNSIGNED_BYTE) {
            uint8_t d = 0xFF;
            data = &d;
        }
        else if (format.mType == GL_FLOAT) {
            float d = 1.f;
            data = &d;
        }
        else if (format.mType == GL_INT) {
            int d = 255;
            data = &d;
        }
        else {
            NEO_ASSERT(false, "Trying to create an empty texture with an unsupported type");
        }
        Texture* t = new T(format, size, data);
        _insertTexture(name, t);
        return t;
    }


}



