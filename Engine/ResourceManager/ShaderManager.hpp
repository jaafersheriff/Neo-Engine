#pragma once

#include "ResourceManagerInterface.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"

#include "Jobs/JobSystem.hpp"

#include "Util/Util.hpp"

#include <array>
#include <chrono>
#include <mutex>
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

		// Quiesces the hot reload sweep and releases its handle. Called by ResourceManagers::_clear
		// before the cache is torn down, and it has to happen there rather than in the destructor: the
		// JobSystem is already gone by the time the resource managers die, and ~JobHandle waits on it.
		void waitForHotReload();

	protected:
		[[nodiscard]] ShaderHandle _asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, const std::optional<std::string>& debugName) const;
		void _destroyImpl(CachedResource<SourceShader>& sourceShader);
		void _initImpl();
		void _tickImpl();
	private:
		std::mutex mHotReloadMutex;

		// Reused across sweeps so the poll does not allocate. Only ever touched by the sweep, and only
		// one sweep exists at a time - the next is not dispatched until the last has completed, which
		// is also what publishes its writes to whichever worker picks up the next one.
		struct ReloadCandidate {
			ShaderHandle mHandle;
			SourceShader::ConstructionArgs mConstructionArgs;
			time_t mModifiedTime;
			std::string mName;
		};
		std::vector<ReloadCandidate> mReloadCandidates;

		// The sweep is an ordinary low-priority job re-dispatched from _tickImpl once the interval has
		// elapsed, so the frame loop supplies the cadence that a sleeping thread used to. Nothing sleeps
		// on a worker: a task that sleeps in a loop is a worker taken out of circulation.
		JobHandle mHotReloadJob;
		std::chrono::steady_clock::time_point mLastHotReloadSweep = {};
		void _hotReloadSweep();
		void _kickHotReload();
	};
}