#pragma once

namespace neo {
	struct ShaderDefinesFG {
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
			memcpy(b, define.mVal.c_str(), define.mVal.size());
			mDefines[mDefinesIndex++] = b;
		}

		void toOldStyle(ShaderDefines& defines) const {
			for (int i = 0; i < mDefinesIndex; i++) {
				defines.set(mDefines[i]);
			}
		}

	private:
		const char* mDefines[16];
		uint8_t mDefinesIndex = 0;
	};

}