#include "Loader/pch.hpp"
#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "Engine/ImGuiManager.hpp"

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
        return {};
    }

    MeshData Library::loadMesh(const std::string& fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Library", "loadMesh", MP_AUTO);

        auto it = mMeshes.find(fileName);
        if (it != mMeshes.end()) {
            return it->second;
        }

        NEO_LOG("First request for %s, loading...", fileName.c_str());
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
        return nullptr;
    }

    Texture* Library::loadTexture(const std::string& fileName, TextureFormat format) {
        MICROPROFILE_SCOPEI("Library", "loadTexture", MP_AUTO);

        auto it = mTextures.find(fileName);
        if (it != mTextures.end()) {
            return it->second;
        }

        NEO_LOG("First request for %s, loading...", fileName.c_str());
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
        NEO_LOG("Creating FBO %s", name.c_str());
        return fb;
    }

    Framebuffer* Library::getFBO(const std::string &name) {
        MICROPROFILE_SCOPEI("Library", "getFBO", MP_AUTO);
        auto it = mFramebuffers.find(name);
        if (it != mFramebuffers.end()) {
            return it->second;
        }

        NEO_LOG_E("FBO %s doesn't exist", name.c_str());
        return nullptr;
    }

    void Library::_insertMesh(const std::string& name, MeshData data) {
        if (data.mMesh) {
            mMeshes.insert({ name, data });
        }
    }

    void Library::_insertTexture(const std::string& name, Texture* texture) {
        if (texture) {
            mTextures.insert({ name, texture });
        }
    }

    void Library::clean() {
        NEO_LOG("Cleaning library...");
        // Clean up GL objects
        for (auto& meshData : mMeshes) {
            meshData.second.mMesh->destroy();
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

    void Library::imGuiEditor() {
        auto textureFunc = [&](const Texture& texture) {
            float scale = 150.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
#pragma warning(push)
#pragma warning(disable: 4312)
                ImGui::Image(reinterpret_cast<ImTextureID>(texture.mTextureID), ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
            };

        ImGui::Begin("Library");
        if (ImGui::TreeNodeEx("FBOs", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& fbo : Library::mFramebuffers) {
                ImGui::TextWrapped((fbo.first + " (" + std::to_string(fbo.second->mFBOID) + ")").c_str());
                for (auto& t : fbo.second->mTextures) {
                    ImGui::SameLine();
                    ImGui::TextWrapped((std::to_string(t->mTextureID) + " [" + std::to_string(t->mWidth) + ", " + std::to_string(t->mHeight) + "]").c_str());
                    ImGui::SameLine();
                    textureFunc(*t);
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& m : Library::mMeshes) {
                ImGui::TextWrapped("%s", m.first.c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& t : Library::mTextures) {
                if (ImGui::TreeNode((t.first + " (" + std::to_string(t.second->mTextureID) + ")" + " [" + std::to_string(t.second->mWidth) + ", " + std::to_string(t.second->mHeight) + "]").c_str())) {
                    // TODO - only run this on 2D textures
                    textureFunc(*t.second);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::End();

    }
}
