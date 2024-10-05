#pragma once

#include "ResourceManagerInterface.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <thread>
#include <variant>

namespace neo {
	class ResourceManagers;

	using ShaderHandle = ResourceHandle<SourceShader>;
	using ShaderLoadDetails = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;

	class ShaderManager final : public ResourceManagerInterface<ShaderManager, SourceShader, ShaderLoadDetails> {
		friend ResourceManagerInterface;
	public:

		ShaderManager();
		~ShaderManager();

		const ResolvedShaderInstance& ShaderManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const;
		void imguiEditor();

	protected:
		[[nodiscard]] ShaderHandle _asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, const std::optional<std::string>& debugName) const;
		void _destroyImpl(BackedResource<SourceShader>& sourceShader);
		void _tickImpl();
	private:
		std::thread* mHotReloader;
		std::atomic<bool> mKillSwitch = false;
		void _hotReloadFunc();
	};
}