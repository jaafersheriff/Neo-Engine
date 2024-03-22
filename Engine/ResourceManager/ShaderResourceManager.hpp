#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>

#include <variant>
#include <memory>

namespace neo {
	class ResourceManagers;

	using ShaderHandle = entt::id_type;
	class ShaderResourceManager {
		friend ResourceManagers;
	public:
		ShaderResourceManager();
		~ShaderResourceManager();
		using ShaderLoadDetails = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;

		const ResolvedShaderInstance& get(HashedString id, const ShaderDefines& defines) const;
		const ResolvedShaderInstance& get(ShaderHandle handle, const ShaderDefines& defines) const;

		bool isValid(ShaderHandle id) const;
		[[nodiscard]] ShaderHandle asyncLoad(const char* name, ShaderLoadDetails meshDetails) const;

		void clear();
		void imguiEditor();
	private:
		void _tick();
		mutable std::map<std::string, ShaderLoadDetails> mQueue;
		using ShaderCache = entt::resource_cache<SourceShader>;
		ShaderCache mShaderCache;
		std::shared_ptr<SourceShader> mDummyShader;
	};
}