#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include "Loader.hpp"

#include "Util/Util.hpp"

#include <unordered_map>

namespace neo {

    class Engine;
    class Framebuffer;

    class Library {

        friend Engine;

        public:
            Library() = default;
            ~Library() = default;
            Library(const Library&) = delete;
            Library& operator=(const Library&) = delete;

            static void clean();

            static MeshData getMesh(const std::string&);
            static MeshData loadMesh(const std::string&, bool = false);
            static void insertMesh(const std::string&, MeshData& mesh);
            static const std::unordered_map<std::string, MeshData> getAllMeshes() { return mMeshes; }

            template <typename T>
            static Texture* createEmptyTexture(const std::string&, TextureFormat, glm::uvec3 = glm::uvec3(1));
            static Texture* getTexture(const std::string&);
            static Texture* loadTexture(const std::string&, TextureFormat = TextureFormat{});
            static Texture* loadCubemap(const std::string&, const std::vector<std::string> &);

            static Framebuffer* createFBO(const std::string&);
            static Framebuffer* getFBO(const std::string&);

            static void imGuiEditor();

        private:
            // TODO 
            // template <typename T> static T* _find(const std::string&);
            static std::unordered_map<std::string, MeshData> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;

            static void _insertMesh(const std::string&, MeshData);
            static void _insertTexture(const std::string&, Texture*);
    };


    template <typename T>
    Texture* Library::createEmptyTexture(const std::string& name, TextureFormat format, glm::uvec3 size) {
        static_assert(std::is_base_of<Texture, T>::value, "T must be a Texture type");
        static_assert(!std::is_same<T, Texture>::value, "T must be a derived Texture type");

        auto it = mTextures.find(name);
        NEO_ASSERT(it == mTextures.end(), "Texture already found");
        Texture* t = new T(format, size, nullptr);
        _insertTexture(name, t);
        return t;
    }
}



