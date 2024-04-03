#pragma once

#include "ResourceManagerInterface.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <variant>

namespace neo {
	class ResourceManagers;

	using ShaderLoadDetails = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;

	class ShaderResourceManager final : public ResourceManagerInterface<ShaderResourceManager, SourceShader, ShaderLoadDetails> {
		friend ResourceManagerInterface;
	public:
		using ShaderHandle = ResourceHandle<SourceShader>;

		ShaderResourceManager();
		~ShaderResourceManager();

		const ResolvedShaderInstance& ShaderResourceManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const;
		void imguiEditor();

	protected:
		[[nodiscard]] ShaderHandle _asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, std::optional<std::string> debugName) const;
		void _clearImpl();
		void _tickImpl();


	};
}