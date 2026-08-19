#pragma once

#include "ResourceManagerInterface.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Util/Util.hpp"

#include <array>
#include <thread>
#include <variant>

namespace neo {
	class ResourceManagers;

	struct ShaderBuilder {
	public:
		ShaderBuilder& setStage(types::shader::Stage stage, std::string src) {
			mConstructionArgs[toIndex(stage)] = src;
			return *this;
		}

		static constexpr size_t toIndex(types::shader::Stage s) {
			return static_cast<size_t>(s);
		}

		using ConstructionArgs = std::array<std::string, static_cast<size_t>(types::shader::Stage::COUNT)>;
		ConstructionArgs mConstructionArgs;
	};
	using ShaderLoadDetails = std::variant<ShaderBuilder, SourceShader::ShaderCode>;
	using ShaderHandle = ResourceHandle<SourceShader>;

	class ShaderManager final : public ResourceManagerInterface<ShaderManager, SourceShader, ShaderLoadDetails> {
		friend ResourceManagerInterface;
	public:

		ShaderManager();
		~ShaderManager();

		const ResolvedShaderInstance& ShaderManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const;
		void imguiEditor();

	protected:
		[[nodiscard]] ShaderHandle _asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, const std::optional<std::string>& debugName) const;
		void _destroyImpl(CachedResource<SourceShader>& sourceShader);
		void _tickImpl();
	private:
		std::thread* mHotReloader;
		std::atomic<bool> mKillSwitch = false;
		void _hotReloadFunc();
	};
}