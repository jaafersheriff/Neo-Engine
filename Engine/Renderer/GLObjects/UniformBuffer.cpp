#include "UniformBuffer.hpp"

namespace neo {

	void UniformBuffer::destroy() {
		mUniformIndex = mTextureIndex = 0;
	}

	void UniformBuffer::bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
		Uniform& uniform = mUniforms[mUniformIndex++];
		uniform.mStringHash = HashedString(name).value();

		util::visit(variant, 
			[&](bool b) { 
				memcpy(reinterpret_cast<bool*>(uniform.mData), &b, sizeof(bool));
				uniform.mType = UniformType::Bool;
			},
			[&](int i) { 
				memcpy(reinterpret_cast<int*>(uniform.mData), &i, sizeof(int));
				uniform.mType = UniformType::Int;
			},
			[&](uint32_t i) { 
				memcpy(reinterpret_cast<uint32_t*>(uniform.mData), &i, sizeof(uint32_t));
				uniform.mType = UniformType::Uint32_t;
			},
			[&](double d) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &d, sizeof(double));
				uniform.mType = UniformType::Double;
			},
			[&](float f) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &f, sizeof(float ));
				uniform.mType = UniformType::Float;
			},
			[&](glm::vec2 v) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &v, sizeof(glm::vec2 ));
				uniform.mType = UniformType::Vec2;
			},
			[&](glm::ivec2 v) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &v, sizeof(glm::ivec2 ));
				uniform.mType = UniformType::Ivec2;
			},
			[&](glm::uvec2 v) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &v, sizeof(glm::uvec2 ));
				uniform.mType = UniformType::Uvec2;
			},
			[&](glm::vec3 v) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &v, sizeof(glm::vec3 ));
				uniform.mType = UniformType::Vec3;
			},
			[&](glm::vec4 v) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &v, sizeof(glm::vec4 ));
				uniform.mType = UniformType::Vec4;
			},
			[&](glm::mat3 m) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &m, sizeof(glm::mat3 ));
				uniform.mType = UniformType::Mat3;
			},
			[&](glm::mat4 m) { 
				memcpy(reinterpret_cast<double*>(uniform.mData), &m, sizeof(glm::mat4 ));
				uniform.mType = UniformType::Mat4;
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);
	}

	void UniformBuffer::bindTexture(const char* name, TextureHandle handle) {
		mTextures[mTextureIndex++] = TextureBind{
			HashedString(name).value(),
			handle
		};
	}

	std::pair<entt::id_type, ResolvedShaderInstance::UniformVariant> UniformBuffer::getUniform(uint8_t index) const {
		NEO_ASSERT(mUniformIndex > index, "heh?");
		const Uniform& uniform = mUniforms[index];
		switch (uniform.mType) {
		case UniformType::Bool:
			return { uniform.mStringHash, *(reinterpret_cast<const bool*>(uniform.mData)) };
			break;
		case UniformType::Int:
			return { uniform.mStringHash, *(reinterpret_cast<const int*>(uniform.mData)) };
			break;
		case UniformType::Uint32_t:
			return { uniform.mStringHash, *(reinterpret_cast<const uint32_t*>(uniform.mData)) };
			break;
		case UniformType::Double:
			return { uniform.mStringHash, *(reinterpret_cast<const double*>(uniform.mData)) };
			break;
		case UniformType::Float:
			return { uniform.mStringHash, *(reinterpret_cast<const float*>(uniform.mData)) };
			break;
		case UniformType::Vec2:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::vec2*>(uniform.mData)) };
			break;
		case UniformType::Ivec2:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::ivec2*>(uniform.mData)) };
			break;
		case UniformType::Uvec2:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::uvec2*>(uniform.mData)) };
			break;
		case UniformType::Vec3:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::vec3*>(uniform.mData)) };
			break;
		case UniformType::Vec4:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::vec4*>(uniform.mData)) };
			break;
		case UniformType::Mat3:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::mat3*>(uniform.mData)) };
			break;
		case UniformType::Mat4:
			return { uniform.mStringHash, *(reinterpret_cast<const glm::mat4*>(uniform.mData)) };
			break;
		default:
			NEO_FAIL("Invalid");
			return {};
		}
	}

	std::pair<entt::id_type, TextureHandle> UniformBuffer::getTexture(uint8_t index) const {
		NEO_ASSERT(mTextureIndex > index, "heh?");
		return { mTextures[index].mStringHash, mTextures[index].mHandle };

	}
}