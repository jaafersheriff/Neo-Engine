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
    std::unordered_map<neo::PooledFramebufferDetails, std::vector<Library::PooledFramebuffer>> Library::mPooledFramebuffers;
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
        for (auto it = mPooledFramebuffers.begin(); it != mPooledFramebuffers.end();) {
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
                it = mPooledFramebuffers.erase(it);
            }
            if (it != mPooledFramebuffers.end()) {
                it++;
            }
        }
    }

    MeshData Library::getMesh(const std::string& name) {
        TRACY_ZONE();
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
        TRACY_ZONE();
        auto it = mTextures.find(name);
        if (it != mTextures.end()) {
            return it->second;
        }

        NEO_FAIL("Texture %s not found", name.c_str());
        return nullptr;
    }

    bool Library::hasTexture(const std::string& fileName) {
        return mTextures.find(fileName) != mTextures.end();
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
        fb->init();
        mFramebuffers.emplace(name, fb);
        NEO_LOG("Creating FBO %s", name.c_str());
        return fb;
    }

    Framebuffer* Library::getPooledFramebuffer(const PooledFramebufferDetails& details, std::optional<std::string> name) {
        TRACY_ZONE();
        PooledFramebuffer& pfb = _findPooledFramebuffer(details);
        pfb.mUsedThisFrame = true;
        pfb.mName = name;
        if (!pfb.mFramebuffer) {
            pfb.mFramebuffer = new Framebuffer;
            pfb.mFramebuffer->init();
            for (auto& format : details.mFormats) {
                if (format.mBaseFormat == GL_DEPTH_COMPONENT) {
                    pfb.mFramebuffer->attachDepthTexture(details.mSize, format.mInternalFormat, format.mFilter, format.mMode);
                }
                else if (format.mBaseFormat == GL_DEPTH_STENCIL) {
                    pfb.mFramebuffer->attachStencilTexture(details.mSize, format.mFilter, format.mMode);
                }
                else {
                    pfb.mFramebuffer->attachColorTexture(details.mSize, format);
                }
            }
            if (pfb.mFramebuffer->mColorAttachments) {
                pfb.mFramebuffer->initDrawBuffers();
            }
            pfb.mFrameCount = 1;
        }

        if (pfb.mFrameCount < 5) {
            pfb.mFrameCount++;
        }

        return pfb.mFramebuffer;
    }

    Library::PooledFramebuffer& Library::_findPooledFramebuffer(const PooledFramebufferDetails& details) {
        TRACY_ZONE();
        auto it = mPooledFramebuffers.find(details);

        // First time seeing this description
        if (it == mPooledFramebuffers.end()) {
            mPooledFramebuffers[details] = {};
            return mPooledFramebuffers[details].emplace_back(PooledFramebuffer{});
        }
        else {
            // There's already a list here, search it
            for (auto& existingFramebuffer : it->second) {
                // An unused resource exists
                if (!existingFramebuffer.mUsedThisFrame) {
                    return existingFramebuffer;
                }
            }
            // No unused resources :( Make a new one
            return it->second.emplace_back(PooledFramebuffer{});
        }
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
            float scale = 175.f / (texture.mWidth > texture.mHeight ? texture.mWidth : texture.mHeight);
#pragma warning(push)
#pragma warning(disable: 4312)
                ImGui::Image(reinterpret_cast<ImTextureID>(texture.mTextureID), ImVec2(scale * texture.mWidth, scale * texture.mHeight), ImVec2(0, 1), ImVec2(1, 0));
#pragma warning(pop)
            };

        ImGui::Begin("Library");

        if (ImGui::TreeNodeEx("Framebuffers", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("##Framebuffers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Name/Size");
                ImGui::TableSetupColumn("Attachments");
                ImGui::TableHeadersRow();
                for (auto& fbo : Library::mFramebuffers) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", fbo.first.c_str());
                    ImGui::Text("[%d, %d]", fbo.second->mTextures[0]->mWidth, fbo.second->mTextures[0]->mHeight);
                    ImGui::TableSetColumnIndex(1);
                    for (auto& t : fbo.second->mTextures) {
                        textureFunc(*t);
                        ImGui::SameLine();
                    }
                }
                for (auto& [hash, tvList] : Library::mPooledFramebuffers) {
                    for (auto& tv : tvList) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        if (tv.mName) {
                            ImGui::Text("*%s", tv.mName.value().c_str());
                        }
                        ImGui::Text("[%d, %d]", tv.mFramebuffer->mTextures[0]->mWidth, tv.mFramebuffer->mTextures[0]->mHeight);
                        ImGui::TableSetColumnIndex(1);
                        for (auto& t : tv.mFramebuffer->mTextures) {
                            textureFunc(*t);
                            ImGui::SameLine();
                        }
                    }
                }
                ImGui::EndTable();
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
                if (ImGui::TreeNode(t.first.c_str())) {
                    ImGui::Text("[%d, %d]", t.second->mWidth, t.second->mHeight);
                    textureFunc(*t.second);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::End();

    }
}
