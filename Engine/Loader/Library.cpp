#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

namespace neo {
    /* Library */
    std::unordered_map<std::string, MeshData> Library::mMeshes;
    std::unordered_map<std::string, Texture*> Library::mTextures;
    std::unordered_map<std::string, Framebuffer*> Library::mFramebuffers;

    MeshData Library::getMesh(const std::string& name) {
        /* Search map first */
        auto it = mMeshes.find(name);
        if (it != mMeshes.end()) {
            return it->second;
        }

        NEO_FAIL("Mesh %s not found", name.c_str());
    }

    MeshData Library::loadMesh(const std::string& fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Library", "loadMesh", MP_AUTO);

        auto it = mMeshes.find(fileName);
        if (it != mMeshes.end()) {
            return it->second;
        }

        MeshData mesh = Loader::loadMesh(fileName, doResize);
        _insertMesh(fileName, mesh);
        return mesh;
    }

    void Library::insertMesh(const std::string& name, MeshData& data) {
        _insertMesh(name, data);
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

    void Library::_insertMesh(const std::string& name, MeshData data) {
        if (data.mesh) {
            mMeshes.insert({ name, data });
        }
    }

    void Library::_insertTexture(const std::string& name, Texture* texture) {
        if (texture) {
            mTextures.insert({ name, texture });
        }
    }

    void Library::clean() {
        // Clean up GL objects
        for (auto& meshData : mMeshes) {
            meshData.second.mesh->destroy();
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
