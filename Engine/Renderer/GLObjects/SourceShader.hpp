#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace neo {
    class ResolvedShaderInstance;

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

    struct ShaderDefines {
        ShaderDefines() = default;
        ShaderDefines(const ShaderDefines& parent) 
            : mParent(&parent) {
        }
        ShaderDefines& operator=(const ShaderDefines&) = delete;
        ShaderDefines& operator=(ShaderDefines&&) = delete;

        void set(const ShaderDefine& define) {
            mDefines[define] = true;
        }

        void reset() {
            for (auto& define : mDefines) {
                define.second = false;
            }
        }

        const ShaderDefines* mParent = nullptr;
        std::map<ShaderDefine, bool> mDefines;
    };

	class SourceShader {
    public:
        using ConstructionArgs = std::unordered_map<ShaderStage, std::string>;
        using ShaderCode = std::unordered_map<ShaderStage, const char*>;
        using HashedShaderDefines = uint32_t;

        SourceShader(const char* name, const ConstructionArgs& args);
        SourceShader(const char* name, const ShaderCode& args);
        ~SourceShader();
		SourceShader(const SourceShader &) = delete;
        SourceShader & operator=(const SourceShader &) = delete;
        SourceShader(SourceShader &&) = delete;
        SourceShader & operator=(SourceShader &&) = delete;

        const ResolvedShaderInstance& getResolvedInstance(const ShaderDefines& defines);
        void imguiEditor();
        void destroy();
    private:
        std::string mName;
        std::optional<ConstructionArgs> mConstructionArgs;
        ShaderCode mShaderSources;
        
        // Can't store ShaderDefines in the map because of const char* and mParent*
        // Also this is faster than specialized std::hash
        std::unordered_map<HashedShaderDefines, ResolvedShaderInstance> mResolvedShaders;
        HashedShaderDefines _getDefinesHash(const ShaderDefines& defines);
	};
}