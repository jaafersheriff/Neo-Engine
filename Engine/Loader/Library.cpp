#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

namespace neo {
    /* Library */
    std::unordered_map<std::string, Mesh *> Library::mMeshes;
    std::unordered_map<std::string, Texture *> Library::mTextures;
    std::unordered_map<std::string, Framebuffer *> Library::mFramebuffers;

    Mesh* Library::getMesh(const std::string& name) {
        /* Search map first */
        auto it = mMeshes.find(name);
        if (it != mMeshes.end()) {
            return it->second;
        }

        NEO_FAIL("Mesh %s not found", name.c_str());
    }

    Mesh* Library::loadMesh(const std::string& fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Library", "loadMesh", MP_AUTO);

        auto it = mMeshes.find(fileName);
        if (it != mMeshes.end()) {
            return it->second;
        }

        Mesh* mesh = Loader::loadMesh(fileName, doResize);
        _insertMesh(fileName, mesh);
        return mesh;
    }

    void Library::loadMesh(const std::string& name, Mesh* mesh) {
        _insertMesh(name, mesh);
    }

    Mesh* Library::createEmptyMesh(const std::string& name) {
        auto it = mMeshes.find(name);
        NEO_ASSERT(it == mMeshes.end(), "Mesh already found");
        Mesh* mesh = new Mesh;
        _insertMesh(name, mesh);
        return mesh;

    }

    Texture* Library::getTexture(const std::string& name) {
        auto it = mTextures.find(name);
        if (it != mTextures.end()) {
            return it->second;
        }

        NEO_FAIL("Texture %s not found", name.c_str());
    }

    Texture* Library::loadTexture(const std::string& fileName, TextureFormat format) {
        MICROPROFILE_SCOPEI("Library", "loadTexture", MP_AUTO);

        auto it = mTextures.find(fileName);
        if (it != mTextures.end()) {
            return it->second;
        }

        Texture2D* texture = Loader::loadTexture(fileName, format);
        _insertTexture(fileName, texture);
        return texture;
    }

    Texture* Library::loadCubemap(const std::string& name, const std::vector<std::string> &files) {
        MICROPROFILE_SCOPEI("Library", "loadCubemap", MP_AUTO);

        auto texture = Loader::loadTexture(name, files);
        _insertTexture(name, texture);

        return texture;
    }

    Framebuffer* Library::createFBO(const std::string& name) {
        auto fb = new Framebuffer;
        mFramebuffers.emplace(name, fb);
        return fb;
    }

    Framebuffer* Library::getFBO(const std::string &name) {
        MICROPROFILE_SCOPEI("Library", "getFBO", MP_AUTO);
        auto it = mFramebuffers.find(name);
        if (it != mFramebuffers.end()) {
            return it->second;
        }

        NEO_FAIL("FBO %s doesn't exist", name.c_str());
        return nullptr;
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

    void Library::clean() {
        // Clean up GL objects
        for (auto& mesh : mMeshes) {
            mesh.second->destroy();
        }
        mMeshes.clear();
        for (auto& texture : mTextures) {
            texture.second->destroy();
        }
        mTextures.clear();
        for (auto& frameBuffer : mFramebuffers) {
            frameBuffer.second->destroy();
        }
        mFramebuffers.clear();
    }
}
