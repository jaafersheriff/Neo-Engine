#include "UniformBuffer.hpp"

namespace neo {
	UniformBuffer::UniformBuffer() 
		: mUniformIndex(0)
		, mTextureIndex(0)
	{
		TRACY_ZONE();
	}

	void UniformBuffer::destroy() {
		for (int i = 0; i < mUniformIndex; i++) {
			switch (mUniforms[i].mType) {
			case UniformType::Bool:
				delete reinterpret_cast<bool*>(mUniforms[i].mData);
				break;
			case UniformType::Int:
				delete reinterpret_cast<int*>(mUniforms[i].mData);
				break;
			case UniformType::Uint32_t:
				delete reinterpret_cast<uint32_t*>(mUniforms[i].mData);
				break;
			case UniformType::Double:
				delete reinterpret_cast<double*>(mUniforms[i].mData);
				break;
			case UniformType::Float:
				delete reinterpret_cast<float*>(mUniforms[i].mData);
				break;
			case UniformType::Vec2:
				delete reinterpret_cast<glm::vec2*>(mUniforms[i].mData);
				break;
			case UniformType::Ivec2:
				delete reinterpret_cast<glm::ivec2*>(mUniforms[i].mData);
				break;
			case UniformType::Uvec2:
				delete reinterpret_cast<glm::uvec2*>(mUniforms[i].mData);
				break;
			case UniformType::Vec3:
				delete reinterpret_cast<glm::vec3*>(mUniforms[i].mData);
				break;
			case UniformType::Vec4:
				delete reinterpret_cast<glm::vec4*>(mUniforms[i].mData);
				break;
			case UniformType::Mat3:
				delete reinterpret_cast<glm::mat3*>(mUniforms[i].mData);
				break;
			case UniformType::Mat4:
				delete reinterpret_cast<glm::mat4*>(mUniforms[i].mData);
				break;
			default:
				NEO_FAIL("Invalid");
			}
		}
	}

	void UniformBuffer::bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
		void* data;
		UniformType type;
		util::visit(variant, 
			[&](bool b) { 
				data = reinterpret_cast<void*>(new bool(b));
				type = UniformType::Bool;
			},
			[&](int i) { 
				data = reinterpret_cast<void*>(new int(i));
				type = UniformType::Int;
			},
			[&](uint32_t i) { 
				data = reinterpret_cast<void*>(new uint32_t(i));
				type = UniformType::Uint32_t;
			},
			[&](double d) { 
				data = reinterpret_cast<void*>(new double(d));
				type = UniformType::Double;
			},
			[&](float f) { 
				data = reinterpret_cast<void*>(new float(f));
				type = UniformType::Float;
			},
			[&](glm::vec2 v) { 
				data = reinterpret_cast<void*>(new glm::vec2(v));
				type = UniformType::Vec2;
			},
			[&](glm::ivec2 v) { 
				data = reinterpret_cast<void*>(new glm::ivec2(v));
				type = UniformType::Ivec2;
			},
			[&](glm::uvec2 v) { 
				data = reinterpret_cast<void*>(new glm::uvec2(v));
				type = UniformType::Uvec2;
			},
			[&](glm::vec3 v) { 
				data = reinterpret_cast<void*>(new glm::vec3(v));
				type = UniformType::Vec3;
			},
			[&](glm::vec4 v) { 
				data = reinterpret_cast<void*>(new glm::vec4(v));
				type = UniformType::Vec4;
			},
			[&](glm::mat3 m) { 
				data = reinterpret_cast<void*>(new glm::mat3(m));
				type = UniformType::Mat3;
			},
			[&](glm::mat4 m) { 
				data = reinterpret_cast<void*>(new glm::mat4(m));
				type = UniformType::Mat4;
			},
			[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
		);

		mUniforms[mUniformIndex++] = Uniform{
			HashedString(name).value(),
			type, 
			data
		};
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
			return { uniform.mStringHash, *(reinterpret_cast<bool*>(uniform.mData)) };
			break;
		case UniformType::Int:
			return { uniform.mStringHash, *(reinterpret_cast<int*>(uniform.mData)) };
			break;
		case UniformType::Uint32_t:
			return { uniform.mStringHash, *(reinterpret_cast<uint32_t*>(uniform.mData)) };
			break;
		case UniformType::Double:
			return { uniform.mStringHash, *(reinterpret_cast<double*>(uniform.mData)) };
			break;
		case UniformType::Float:
			return { uniform.mStringHash, *(reinterpret_cast<float*>(uniform.mData)) };
			break;
		case UniformType::Vec2:
			return { uniform.mStringHash, *(reinterpret_cast<glm::vec2*>(uniform.mData)) };
			break;
		case UniformType::Ivec2:
			return { uniform.mStringHash, *(reinterpret_cast<glm::ivec2*>(uniform.mData)) };
			break;
		case UniformType::Uvec2:
			return { uniform.mStringHash, *(reinterpret_cast<glm::uvec2*>(uniform.mData)) };
			break;
		case UniformType::Vec3:
			return { uniform.mStringHash, *(reinterpret_cast<glm::vec3*>(uniform.mData)) };
			break;
		case UniformType::Vec4:
			return { uniform.mStringHash, *(reinterpret_cast<glm::vec4*>(uniform.mData)) };
			break;
		case UniformType::Mat3:
			return { uniform.mStringHash, *(reinterpret_cast<glm::mat3*>(uniform.mData)) };
			break;
		case UniformType::Mat4:
			return { uniform.mStringHash, *(reinterpret_cast<glm::mat4*>(uniform.mData)) };
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