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
		HashedString mVal;

		friend bool operator<(const ShaderDefine& l, const ShaderDefine& r) {
			return l.mVal.value() < r.mVal.value();
		}
	};
}