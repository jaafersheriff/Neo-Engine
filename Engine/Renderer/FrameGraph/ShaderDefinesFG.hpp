#pragma once

namespace neo {
	struct ShaderDefinesFG {
		using HashedShaderDefines = ENTT_ID_TYPE;
		static HashedShaderDefines getDefinesHash(const ShaderDefinesFG& define, HashedShaderDefines seed = 0) {
			seed = define.getDefinesSize() ^ seed;
			for (uint8_t i = 0; i < define.getDefinesSize(); i++) {
				seed ^= HashedString(define.getDefine(i)).value() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

		static HashedShaderDefines getDefinesHash(const std::vector<ShaderDefinesFG>& defines) {
			HashedShaderDefines seed = static_cast<HashedShaderDefines>(defines.size());
			for (const auto& define : defines) {
				seed ^= getDefinesHash(define, seed);
			}
			return seed;
		}

		void destroy() {
			for (int i = 0; i < mDefinesIndex; i++) {
				delete mDefines[i];
			}
			mDefinesIndex = 0;
		}

		uint8_t getDefinesSize() const { return mDefinesIndex; }
		const char* getDefine(uint8_t index) const { 
			NEO_ASSERT(index < mDefinesIndex, "Invalid index");
			return mDefines[index];
		}

		void operator=(const ShaderDefinesFG& other) {
			for (uint8_t i = 0; i < other.getDefinesSize(); i++) {
				set(other.getDefine(i));
			}
		}

		void set(const char* define) {
			size_t l = strlen(define);
			char* b = new char[l + 1] {};
			memcpy(b, define, l);
			mDefines[mDefinesIndex++] = b;
		}


		void set(const ShaderDefine& define) {
			char* b = new char[define.mVal.size() + 1] {};
			memcpy(b, define.mVal, define.mVal.size());
			mDefines[mDefinesIndex++] = b;
		}

	private:
		const char* mDefines[16];
		uint8_t mDefinesIndex = 0;
	};

}