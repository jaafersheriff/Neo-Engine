#include "Loader/pch.hpp"
#include "Library.hpp"

#include "Loader.hpp"

#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "Engine/ImGuiManager.hpp"

#include "Util/Profiler.hpp"

namespace neo {
    /* Library */
    std::unordered_map<std::string, MeshData> Library::mMeshes;
    std::unordered_map<std::string, Texture*> Library::mTextures;
    std::unordered_map<std::string, Framebuffer*> Library::mFramebuffers;
    std::unordered_map<neo::Library::HashedTempFramebuffer, std::vector<Library::TempFramebuffer>> Library::mTemporaryFramebuffers;
    std::unordered_map<std::string, SourceShader*> Library::mShaders;
    SourceShader* Library::mDummyShader;

    void Library::init() {
        mDummyShader = new SourceShader("Dummy", SourceShader::ShaderCode{
            {ShaderStage::VERTEX, 
                R"(
                    void main() {
                        gl_Position = vec4(0,0,0,0);
                    }
                )"},
            {ShaderStage::FRAGMENT,
                R"(
                    out vec4 color;
                    void main() {
                        color = vec4(0,0,0,0);
                    }
                )"}
        });
    }

    const ResolvedShaderInstance& Library::getDummyShader() {
        return mDummyShader->getResolvedInstance({});
    }

    void Library::tick() {
        TRACY_ZONE();
        for (auto it = mTemporaryFramebuffers.begin(); it != mTemporaryFramebuffers.end();) {
            auto& tvList = it->second;
            for (auto tvIt = tvList.begin(); tvIt != tvList.end();) {
                if (tvIt->mFrameCount == 0) {
                    tvIt->mFramebuffer->destroy();
                    delete tvIt->mFramebuffer;
                    tvIt = tvList.erase(tvIt);
                }
                else {
                    if (tvIt->mUsedThisFrame) {
                        tvIt->mUsedThisFrame = false;
                    }
                    else {
                        tvIt->mFrameCount--;
                    }
                    tvIt++;
                }
            }

            if (tvList.empty()) {
                it = mTemporaryFramebuffers.erase(it);
            }
            if (it != mTemporaryFramebuffers.end()) {
                it++;
            }
        }
    }

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
        TRACY_ZONE();

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
        TRACY_ZONE();

        auto it = mTextures.find(fileName);
        if (it != mTextures.end()) {
            return it->second;
        }

        NEO_LOG("First request for %s, loading...", fileName.c_str());
        Texture* texture = Loader::loadTexture(fileName, format);
        _insertTexture(fileName, texture);
        return texture;
    }

    Texture* Library::createTexture(const std::string& fileName, TextureFormat format, glm::u16vec3 dimension, const void* data) {
        TRACY_ZONE();

        auto it = mTextures.find(fileName);
        if (it != mTextures.end()) {
            return it->second;
        }

        if (data != nullptr) {
            NEO_LOG("Uploading %s", fileName.c_str());
        }
        Texture* texture = new Texture(format, dimension, data);
        _insertTexture(fileName, texture);
        return texture;
    }

    Texture* Library::loadCubemap(const std::string& name, const std::vector<std::string> &files) {
        TRACY_ZONE();

        Texture* texture = Loader::loadTexture(name, files);
        _insertTexture(name, texture);

        return texture;
    }

    Framebuffer* Library::createFramebuffer(const std::string& name) {
        auto fb = new Framebuffer;
        mFramebuffers.emplace(name, fb);
        NEO_LOG("Creating FBO %s", name.c_str());
        return fb;
    }

    Framebuffer* Library::createTempFramebuffer(glm::uvec2 size, const std::vector<TextureFormat>& formats) {
        TRACY_ZONE();
        TempFramebuffer& tv = _findTempFramebuffer(size, formats);
        tv.mUsedThisFrame = true;
        if (!tv.mFramebuffer) {
            tv.mFramebuffer = new Framebuffer;
            for (auto& format : formats) {
                if (format.mBaseFormat == GL_DEPTH_COMPONENT) {
                    tv.mFramebuffer->attachDepthTexture(size, format.mInternalFormat, format.mFilter, format.mMode);
                }
                else if (format.mBaseFormat == GL_DEPTH_STENCIL) {
                    tv.mFramebuffer->attachStencilTexture(size, format.mFilter, format.mMode);
                }
                else {
                    tv.mFramebuffer->attachColorTexture(size, format);
                }
            }
            if (tv.mFramebuffer->mColorAttachments) {
                tv.mFramebuffer->initDrawBuffers();
            }
            tv.mFrameCount = 1;
        }

        if (tv.mFrameCount < 5) {
            tv.mFrameCount++;
        }

        return tv.mFramebuffer;
    }

    Library::TempFramebuffer& Library::_findTempFramebuffer(glm::uvec2 size, const std::vector<TextureFormat>& formats) {
        TRACY_ZONE();
        HashedTempFramebuffer hash = _getTempFramebufferHash(size, formats);
        auto it = mTemporaryFramebuffers.find(hash);

        // First time seeing this description
        if (it == mTemporaryFramebuffers.end()) {
            mTemporaryFramebuffers[hash] = {};
            return mTemporaryFramebuffers[hash].emplace_back(TempFramebuffer{});
        }
        else {
            // There's already a list here, search it
            for (auto& existingTransient : it->second) {
                // An unused resource exists
                if (!existingTransient.mUsedThisFrame) {
                    return existingTransient;
                }
            }
            // No unused resources :( Make a new one
            return it->second.emplace_back(TempFramebuffer{});
        }
    }

    // This is faster than specialized std::hash
    Library::HashedTempFramebuffer Library::_getTempFramebufferHash(glm::uvec2 size, const std::vector<TextureFormat>& formats) {
        TRACY_ZONE();
        NEO_UNUSED(formats);
        HashedTempFramebuffer seed = size.x + size.y;
		for (auto& i : formats) {
			seed ^= i.mBaseFormat + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= i.mInternalFormat + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= i.mFilter + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= i.mMode + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= i.mType + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
    }

    SourceShader* Library::createSourceShader(const std::string& name, const SourceShader::ConstructionArgs& args) {
        auto it = mShaders.find(name);
        if (it != mShaders.end()) {
            return it->second;
        }

        NEO_LOG("Creating Shader %s", name.c_str());
        SourceShader* source = new SourceShader(name.c_str(), args);
        mShaders.emplace(name, source);
        return source;
    }

    SourceShader* Library::createSourceShader(const std::string& name, const SourceShader::ShaderCode& shaderCode) {
        TRACY_ZONE();
        auto it = mShaders.find(name);
        if (it != mShaders.end()) {
            return it->second;
        }

        NEO_LOG("Creating Shader %s", name.c_str());
        SourceShader* source = new SourceShader(name.c_str(), shaderCode);
        mShaders.emplace(name, source);
        return source;
    }


    SourceShader* Library::getSourceShader(const char* name) {
        TRACY_ZONE();
        auto it = mShaders.find(name);
        NEO_ASSERT(it != mShaders.end(), "Shader %s doesn't exist!", name);

        return it->second;
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
        for (auto& shader : mShaders) {
            shader.second->destroy();
        }
        mShaders.clear();
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
                ImGui::TextWrapped("%s (%d)", fbo.first.c_str(), fbo.second->mFBOID);
                for (auto& t : fbo.second->mTextures) {
                    ImGui::SameLine();
                    ImGui::TextWrapped("%d [%d, %d]", t->mTextureID, t->mWidth, t->mHeight);
                    ImGui::SameLine();
                    textureFunc(*t);
                }
            }
            if (ImGui::TreeNodeEx("Transients", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto& [hash, tvList] : Library::mTemporaryFramebuffers) {
                    for (auto& tv : tvList) {
                        ImGui::TextWrapped("[%d] (%d)", static_cast<int>(tv.mFrameCount), tv.mFramebuffer->mFBOID);
                        for (auto& t : tv.mFramebuffer->mTextures) {
                            ImGui::SameLine();
                            ImGui::TextWrapped("%d [%d, %d]", t->mTextureID, t->mWidth, t->mHeight);
                            ImGui::SameLine();
                            textureFunc(*t);
                        }
                    }
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& shader : Library::mShaders) {
                if (ImGui::TreeNode(shader.first.c_str())) {
                    shader.second->imguiEditor();
                    ImGui::TreePop();
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
