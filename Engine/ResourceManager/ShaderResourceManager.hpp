#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <variant>

namespace neo {
	class ResourceManagers;

	using ShaderLoadDetails = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;
	using ShaderHandle = entt::id_type;

	class ShaderResourceManager final : public ResourceManagerInterface<ShaderResourceManager, ShaderHandle, SourceShader, ShaderLoadDetails> {
		friend ResourceManagerInterface;
	public:
		ShaderResourceManager();
		~ShaderResourceManager();

		const ResolvedShaderInstance& ShaderResourceManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const;
		void imguiEditor();

	protected:
		[[nodiscard]] ShaderHandle _asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, std::string debugName) const;
		void _clearImpl();
		void _tickImpl();


	};
}