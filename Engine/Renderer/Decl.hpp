#pragma once

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace neo {

#define MakeUniform(x) static ShaderUniform x(#x)

	struct ShaderUniform {
		// Uhhhh
		ShaderUniform() :
			mVal(0)
		{}
		ShaderUniform(const char* c) :
			mVal(c)
		{}
		//~ShaderUniform() = default;
		HashedString mVal;

		friend bool operator<(const ShaderUniform& l, const ShaderUniform& r) {
			return l.mVal.value() < r.mVal.value();
		}
	};

	struct ShaderUniforms {
		ShaderUniforms() = default;

		void set(const ShaderUniform& uniformDefine, const ResolvedShaderInstance::UniformVariant& variant) {
			mUniforms.emplace(uniformDefine.mVal.value(), variant);
		}

		// Variant is too big for an array, gotta use a map ;(
		std::map<HashedString::hash_type, ResolvedShaderInstance::UniformVariant> mUniforms;
	};

	struct ShaderTextureBinds {
		ShaderTextureBinds() = default;

		void set(const ShaderUniform& uniformDefine, const TextureHandle& handle) {
			NEO_ASSERT(textureCount < 16, "TODO");
			mTextures[textureCount++] = std::make_pair(uniformDefine.mVal.value(), handle);
		}

		// Stack overflow :( 
		std::array<std::pair<HashedString::hash_type, TextureHandle >, 16> mTextures;
		int textureCount = 0;

	};

	struct ClearCommand {
		FramebufferHandle handle = NEO_INVALID_HANDLE;
		glm::vec4 clearColor = glm::vec4(0.f);
		types::framebuffer::AttachmentBits clearFlags = 0;
	};

	struct CopiedShaderDefines {
		std::vector<std::string> defines;

		void set(const ShaderDefine& define) {
			defines.emplace_back(define.mVal);
		}
	};

	struct Draw {
		CopiedShaderDefines drawDefines = {};
		ShaderUniforms uniforms = {};
		ShaderTextureBinds textures = {};

		MeshHandle mesh = NEO_INVALID_HANDLE;
	};

	enum class BlendState : uint8_t {
		Disabled,

		// glEnable(GL_BLEND);
		// glBlendEquation(GL_FUNC_ADD);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Enabled 
	};

	struct RenderState {
		glm::uvec4 viewport = glm::uvec4(0);
		BlendState blendState = BlendState::Disabled;
	};

	struct RenderPass {
		RenderState state = {};
		FramebufferHandle fboHandle = NEO_INVALID_HANDLE;

		ShaderHandle shaderHandle = NEO_INVALID_HANDLE;
		CopiedShaderDefines passDefines = {};
		ShaderUniforms passUniforms = {};
		ShaderTextureBinds passTextures = {};

		Draw& declareDraw() {
			NEO_ASSERT(drawCount < 1024, "TODO");
			return draws[drawCount++];
		}

		// ARRAY OF STRUCTS AH
		std::array<Draw, 1024> draws = {};
		int drawCount = 0;
	};

	class Decl {
	public:
		ClearCommand& Decl::declareClearCommand(FramebufferHandle handle, glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags);
		RenderPass& declareRenderPass(FramebufferHandle target, glm::uvec4 viewport);

		void execute(const ResourceManagers& resourceManagers);

	private:
		using Commands = std::variant<
			ClearCommand,
			RenderPass
		>;
		std::vector<Commands> mCommands;
	};

}

