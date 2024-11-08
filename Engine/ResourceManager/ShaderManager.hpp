#pragma once

#include "ResourceManagerInterface.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <thread>
#include <variant>

namespace neo {
	class ResourceManagers;

	using ShaderLoadDetails = std::variant<SourceShader::ConstructionArgs, SourceShader::ShaderCode>;

	struct ShaderLoader {
		using result_type = std::shared_ptr<BackedResource<SourceShader>>;
		result_type operator()(const ShaderLoadDetails& shaderDetails, const std::optional<std::string>& debugName) const;
	};

	using ShaderHandle = ResourceHandle<SourceShader>;
	class ShaderManager final : public ResourceManagerInterface<ShaderManager, SourceShader, ShaderLoadDetails, ShaderLoader> {
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