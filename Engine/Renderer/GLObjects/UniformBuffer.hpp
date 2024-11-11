#pragma once

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "ResourceManager/TextureManager.hpp"

namespace neo {
	struct UniformBuffer {
		UniformBuffer();
		void destroy();

		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant);
		void bindTexture(const char* name, TextureHandle handle);

		uint8_t getUniformsSize() const { return mUniformIndex; }
		uint8_t getTextureBindSize() const { return mTextureIndex; }

		std::pair<entt::id_type, ResolvedShaderInstance::UniformVariant> getUniform(uint8_t index) const;
		std::pair<entt::id_type, TextureHandle> getTexture(uint8_t index) const;

	private:
		enum class UniformType {
			Bool,
			Int,
			Uint32_t,
			Double,
			Float,
			Vec2,
			Ivec2,
			Uvec2,
			Vec3,
			Vec4,
			Mat3,
			Mat4
		};
		struct Uniform {
			entt::id_type mStringHash;
			UniformType mType;
			void* mData;
		};
		Uniform mUniforms[64] = {};
		uint8_t mUniformIndex = 0;

		struct TextureBind {
			entt::id_type mStringHash;
			TextureHandle mHandle;
		};
		TextureBind mTextures[64] = {};
		uint8_t mTextureIndex = 0;
	};
}