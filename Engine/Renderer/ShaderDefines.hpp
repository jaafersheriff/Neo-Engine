#pragma once

#include "Util/Util.hpp"

#include <string>
#include <sstream>

namespace neo {

#define MakeDefine(x) static ShaderDefine x(#x)

	struct ShaderDefine {
		ShaderDefine(const char* c) :
			mVal(c)
		{}
		std::string mVal;

		friend bool operator<(const ShaderDefine& l, const ShaderDefine& r) {
			return HashedString(l.mVal.c_str()).value() < HashedString(r.mVal.c_str()).value();
		}
	};

	struct ShaderDefines {
		ShaderDefines() = default;
		ShaderDefines(const ShaderDefines& parent) {
			const ShaderDefines* defines = &parent;
			while (defines) {
				for (auto& [d, b] : defines->mDefines) {
					mDefines[d] = b;
				}
				defines = defines->mParent;
			}
		}
		ShaderDefines operator=(const ShaderDefines&) = delete;
		ShaderDefines operator=(ShaderDefines&&) = delete;
		ShaderDefines& operator=(const ShaderDefines&&) = delete;

		void set(const ShaderDefine& define) {
			mDefines[define] = true;
		}

		void reset() {
			for (auto& define : mDefines) {
				define.second = false;
			}
		}

		operator std::string() const {
			std::stringstream ss;
			if (mParent) {
				ss << std::string(*mParent);
			}
			for (auto& define : mDefines) {
				if (define.second) {
					ss << "\t" << define.first.mVal.data() << "\n";
				}
			}
			return ss.str();
		}

		ShaderDefines* mParent = nullptr;
		std::map<ShaderDefine, bool> mDefines;
	};
}