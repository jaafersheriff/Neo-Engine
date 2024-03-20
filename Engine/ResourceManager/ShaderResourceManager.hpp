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
		using ShaderBuilder = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;

		const ResolvedShaderInstance& get(HashedString id, const ShaderDefines& defines) const;
		const ResolvedShaderInstance& get(ShaderHandle id, const ShaderDefines& defines) const;

		bool isValid(ShaderHandle id) const;
		[[nodiscard]] ShaderHandle asyncLoad(HashedString id, ShaderBuilder meshDetails) const;

		void clear();
	private:
		void _tick();
		mutable std::vector<std::pair<std::string, ShaderBuilder>> mQueue;
		using ShaderCache = entt::resource_cache<SourceShader>;
		ShaderCache mShaderCache;
		std::shared_ptr<SourceShader> mDummyShader;
	};
}