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

	struct PooledFramebufferDetails {
		glm::uvec2 mSize;
		std::vector<TextureFormat> mFormats;

		bool operator==(const PooledFramebufferDetails& other) const {
			// Kinda faulty, but meh
			return other.mSize == mSize && other.mFormats == mFormats;
		}
	};

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
			static MeshData loadMesh_DEPRECATED(const std::string&, bool = false);
			static void insertMesh(const std::string&, MeshData& mesh);
			static const std::unordered_map<std::string, MeshData> getAllMeshes() { return mMeshes; }

			static bool hasTexture(const std::string&); // will go away when proper resource manager happens ;( 
			static Texture* getTexture(const std::string&);
			static Texture* loadTexture(const std::string&, TextureFormat = {});
			static Texture* createTexture(const std::string&, TextureFormat, glm::u16vec3 dimension, const void* data = nullptr);
			static Texture* loadCubemap(const std::string&, const std::vector<std::string> &);

			// This is only being used for the offscreen backbuffer now..
			// getPooledFramebuffer should be extended to accept owned textures..
			static Framebuffer* createFramebuffer(const std::string&);
			static Framebuffer* getPooledFramebuffer(const PooledFramebufferDetails& details, std::optional<std::string> name = std::nullopt);

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
				std::optional<std::string> mName = std::nullopt;
			};
			static std::unordered_map<PooledFramebufferDetails, std::vector<PooledFramebuffer>> mPooledFramebuffers;
			static std::unordered_map<std::string, SourceShader*> mShaders;
			static SourceShader* mDummyShader;

			static void _insertMesh(const std::string&, MeshData);
			static void _insertTexture(const std::string&, Texture*);
			static PooledFramebuffer& _findPooledFramebuffer(const PooledFramebufferDetails& details);
	};
}

namespace std {
	template<> struct hash<neo::PooledFramebufferDetails> {
		size_t operator()(const neo::PooledFramebufferDetails& details) const noexcept {
			size_t seed = details.mSize.x + details.mSize.y;
			for (auto& i : details.mFormats) {
				seed ^= i.mBaseFormat + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				seed ^= i.mInternalFormat + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				seed ^= i.mFilter + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				seed ^= i.mMode + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				seed ^= i.mType + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
}


