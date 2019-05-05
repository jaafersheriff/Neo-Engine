#include "Library.hpp"

#include "Loader.hpp"

#include "GLObjects/Framebuffer.hpp"

namespace neo {
    /* Library */
    std::unordered_map<std::string, Mesh *> Library::mMeshes;
    std::unordered_map<std::string, Texture *> Library::mTextures;
    std::unordered_map<std::string, Framebuffer *> Library::mFramebuffers;

    Mesh* Library::getMesh(const std::string& fileName, bool doResize) {
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

    Framebuffer* Library::getFBO(const std::string &name) {
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


}
