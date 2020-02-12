#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

namespace neo {
    /* Library */
    std::unordered_map<std::string, Mesh *> Library::mMeshes;
    std::unordered_map<std::string, Texture *> Library::mTextures;
    std::unordered_map<std::string, Framebuffer *> Library::mFramebuffers;

    Mesh* Library::getMesh(const std::string& fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Library", "getMesh", MP_AUTO);
        /* Search map first */
        auto it = mMeshes.find(fileName);
        if (it != mMeshes.end()) {
            return it->second;
        }

        /* Create mesh if not found */
        auto mesh = Loader::loadMesh(fileName, doResize);
        _insertMesh(fileName, mesh);

        return mesh;
    }

    Mesh* Library::createEmptyMesh(const std::string& name) {
        auto it = mMeshes.find(name);
        NEO_ASSERT(it == mMeshes.end(), "Mesh already found");
        Mesh* mesh = new Mesh;
        _insertMesh(name, mesh);
        return mesh;

    }

    Texture* Library::getTexture(const std::string& fileName, TextureFormat format) {
        MICROPROFILE_SCOPEI("Library", "getTexture", MP_AUTO);
        auto it = mTextures.find(fileName);
        if (it != mTextures.end()) {
            return it->second;
        }

        auto texture = Loader::loadTexture(fileName, format);
        _insertTexture(fileName, texture);

        return texture;
    }

    Texture* Library::getCubemap(const std::string& name, const std::vector<std::string> &files) {
        MICROPROFILE_SCOPEI("Library", "getCubemap", MP_AUTO);
        auto it = mTextures.find(name);
        if (it != mTextures.end()) {
            return it->second;
        }

        auto texture = Loader::loadTexture(name, files);
        _insertTexture(name, texture);

        return texture;
    }

    Framebuffer* Library::getFBO(const std::string &name) {
        MICROPROFILE_SCOPEI("Library", "getFBO", MP_AUTO);
        auto it = mFramebuffers.find(name);
        if (it == mFramebuffers.end()) {
            mFramebuffers.emplace(name, new Framebuffer);
            it = mFramebuffers.find(name);
        }
        return it->second;
    }

    void Library::_insertMesh(const std::string& name, Mesh* mesh) {
        if (mesh) {
            mMeshes.insert({ name, mesh });
        }
    }

    void Library::_insertTexture(const std::string& name, Texture* texture) {
        if (texture) {
            mTextures.insert({ name, texture });
        }
    }


}
