#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Loader.hpp"

#include <unordered_map>
#include <string>

namespace neo {

    class Engine;
    class Framebuffer;
    class SourceShader;

    class Library {

        friend Engine;

        public:
            Library() = default;
            ~Library() = default;
            Library(const Library&) = delete;
            Library& operator=(const Library&) = delete;

            static void init();
            static void tick();
            static void clean();

            static MeshData getMesh(const std::string&);
            static MeshData loadMesh(const std::string&, bool = false);
            static void insertMesh(const std::string&, MeshData& mesh);
            static const std::unordered_map<std::string, MeshData> getAllMeshes() { return mMeshes; }

            static Texture* getTexture(const std::string&);
            static Texture* loadTexture(const std::string&, TextureFormat = {});
            static Texture* createTexture(const std::string&, TextureFormat, glm::u16vec3 dimension, const void* data = nullptr);
            static Texture* loadCubemap(const std::string&, const std::vector<std::string> &);

            // This is only being used for the offscreen backbuffer now..
            // getPooledFramebuffer should be extended to accept owned textures..
            static Framebuffer* createFramebuffer(const std::string&);
            static Framebuffer* getPooledFramebuffer(glm::uvec2 size, const std::vector<TextureFormat>& formats);

			static SourceShader* createSourceShader(const std::string& name, const SourceShader::ConstructionArgs& args);
			static SourceShader* createSourceShader(const std::string& name, const SourceShader::ShaderCode& shaderCode);
            static SourceShader* getSourceShader(const char* name);
            static const ResolvedShaderInstance& getDummyShader();

            static void imGuiEditor();

        private:
            static std::unordered_map<std::string, MeshData> mMeshes;
            static std::unordered_map<std::string, Texture*> mTextures;
            static std::unordered_map<std::string, Framebuffer*> mFramebuffers;
            struct PooledFramebuffer {
                Framebuffer* mFramebuffer = nullptr;
                uint8_t mFrameCount = 0;
                bool mUsedThisFrame = false;
            };
            using PooledFramebufferHash = uint32_t;
            static std::unordered_map<PooledFramebufferHash, std::vector<PooledFramebuffer>> mPooledFramebuffers;
            static std::unordered_map<std::string, SourceShader*> mShaders;
            static SourceShader* mDummyShader;

            static void _insertMesh(const std::string&, MeshData);
            static void _insertTexture(const std::string&, Texture*);
            static PooledFramebufferHash _getPooledFramebufferHash(glm::uvec2 size, const std::vector<TextureFormat>& formats);
            static PooledFramebuffer& _findPooledFramebuffer(glm::uvec2 size, const std::vector<TextureFormat>& formats);
    };

}



