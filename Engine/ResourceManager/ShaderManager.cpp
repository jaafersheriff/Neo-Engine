#include "ShaderManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"

#include <ext/imgui_incl.hpp>

#define HOT_RELOAD_MILLSECONDS 100

namespace neo {
	struct ShaderLoader final {

		std::optional<CachedResource<SourceShader>> load(const ShaderLoadDetails& shaderDetails, const std::optional<std::string>& debugName) const {
			NEO_ASSERT(debugName.has_value(), "Shaders need to come with a name please");
			NEO_LOG_V("Uploading shader %s", debugName.value().c_str());
			return util::visit(shaderDetails,
				[&](const ShaderBuilder& builder) {
					SourceShader::ShaderCode shaderCode;
					time_t lastModTime = 0;
					bool notCompute = false;
					for (int i = 0; i < static_cast<int>(types::shader::Stage::COUNT); i++) {
						types::shader::Stage stage = static_cast<types::shader::Stage>(i);
						if (!builder.mConstructionArgs[i].empty()) {
							shaderCode.emplace(stage, Loader::loadFileString(builder.mConstructionArgs[i]));
							lastModTime = std::max(lastModTime, Loader::getFileModTime(builder.mConstructionArgs[i]));

							if (stage != types::shader::Stage::Compute) {
								notCompute = true;
							}
							else if (notCompute) {
								NEO_FAIL("Shader %s has a compute shader and non-compute shader", debugName->c_str());
							}
						}
					}

					std::optional<CachedResource<SourceShader>> result(std::in_place, debugName->c_str(), shaderCode);

					result->mResource.mConstructionArgs = SourceShader::ConstructionArgs(builder.mConstructionArgs.begin(), builder.mConstructionArgs.end());
					result->mResource.mModifiedTime = lastModTime;
					return result;
				},
				[&](const SourceShader::ShaderCode& shaderCode) {
					return std::optional<CachedResource<SourceShader>>(std::in_place, debugName->c_str(), shaderCode);
				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	};

	ShaderManager::ShaderManager() = default;

	void ShaderManager::_initImpl() {

		std::optional<CachedResource<SourceShader>> fallback = ShaderLoader{}.load(SourceShader::ShaderCode{
			{types::shader::Stage::Vertex,
				R"(
					void main() {
						gl_Position = vec4(0,0,0,0);
					}
				)"},
			{types::shader::Stage::Fragment,
				R"(
					out vec4 color;
					void main() {
						color = vec4(0,0,0,0);
					}
				)"}
			}, "Dummy");
		NEO_ASSERT(fallback.has_value(), "Failed to load the fallback shader");
		mFallback = std::make_shared<CachedResource<SourceShader>>(std::move(*fallback));

		mFallback->mResource.getResolvedInstance({});
	}

	ShaderManager::~ShaderManager() {
		// No thread to join any more. The sweep was already quiesced by waitForHotReload, back when the
		// JobSystem still existed.
		NEO_ASSERT(!mHotReloadJob.isValid(), "Hot reload sweep outlived the JobSystem - waitForHotReload was skipped");
		mFallback.reset();
	}

	void ShaderManager::waitForHotReload() {
		mHotReloadJob = JobHandle{};
	}

	const ResolvedShaderInstance& ShaderManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const {
		auto& resolved = resolve(handle).getResolvedInstance(defines);
		if (resolved.isValid()) {
			resolved.bind();
			return resolved;
		}
		auto& fallback = mFallback->mResource.getResolvedInstance({});
		fallback.bind();
		return fallback;
	}

	[[nodiscard]] ShaderHandle ShaderManager::_asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, const std::optional<std::string>& debugName) const {
		{
			std::lock_guard<std::mutex> lock(mLoadQueueMutex);
			mLoadQueue.emplace_back(ResourceLoadDetails_Internal{ id, shaderDetails, debugName });
		}
		return id;
	}

	void ShaderManager::_tickImpl() {
		TRACY_ZONE();

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			{
				std::lock_guard<std::mutex> lock(mLoadQueueMutex);
				std::swap(swapQueue, mLoadQueue);
				mLoadQueue.clear();
			}

			for (auto& loadDetails : swapQueue) {
				if (std::optional<CachedResource<SourceShader>> shader = ShaderLoader{}.load(loadDetails.mLoadDetails, loadDetails.mDebugName)) {
					mCache.insert(loadDetails.mHandle, std::move(*shader));
				}
				else {
					NEO_LOG_E("Failed to load shader %s", loadDetails.mDebugName.value_or("").c_str());
				}
				// Published (or failed) - the claim taken in asyncLoad ends here, not when it left the queue.
				_finishPending(loadDetails.mHandle);
			}
		}

		{
			std::vector<ShaderHandle> swapQueue = {};
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				std::swap(swapQueue, mDiscardQueue);
				mDiscardQueue.clear();
			}
			// Excludes the hot-reload sweep, which might be reading these same shaders on a worker
			std::lock_guard<std::mutex> lock(mHotReloadMutex);
			for (auto& id : swapQueue) {
				if (isValid(id)) {
					retire(id);
				}
			}
		}

		NEO_ASSERT(mTransactionQueue.empty(), "Shader transactions unsupported");

		_kickHotReload();
	}

	void ShaderManager::_destroyImpl(CachedResource<SourceShader>& sourceShader) {
		if (sourceShader.mResource.mConstructionArgs.has_value()) {
			for (auto&& [type, charString] : sourceShader.mResource.mShaderSources) {
				delete charString;
			}
		}
		sourceShader.mResource.destroy();
	}

	void ShaderManager::imguiEditor() {
		// Not thread safe.
		mCache.forEach([](const CachedResource<SourceShader>& entry) {
			auto& shader = entry.mResource;
			if (ImGui::TreeNode(shader.mName.c_str())) {
				if (shader.mResolvedShaders.size()) {
					if (ImGui::TreeNode("##idk", "Variants (%d)", static_cast<int>(shader.mResolvedShaders.size()))) {
						ImGui::Separator();
						for (const auto& variant : shader.mResolvedShaders) {
							ImGui::Text("%s", variant.second.variant().size() ? variant.second.variant().c_str() : "No defines");
							ImGui::Separator();
						}
						ImGui::TreePop();
					}
				}
				else {
					ImGui::Text("Variants (0)");
				}
				ImGui::TreePop();
			}
		});
	}

	void ShaderManager::_kickHotReload() {
		// Runs on the render thread, at the end of the render job. One sweep at a time: the handle is
		// how we know the last one landed, and re-dispatching over a live sweep would race it on
		// mReloadCandidates.
		if (mHotReloadJob.isValid() && !mHotReloadJob.isComplete()) {
			return;
		}

		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		if (now - mLastHotReloadSweep < std::chrono::milliseconds(HOT_RELOAD_MILLSECONDS)) {
			return;
		}
		mLastHotReloadSweep = now;

		// Low priority: a mod-time sweep should fill an idle core, never push the frame around.
		mHotReloadJob = ServiceLocator<JobSystem>::ref().dispatch([this] { _hotReloadSweep(); }, JobPriority::Low);
	}

	void ShaderManager::_hotReloadSweep() {
		// Doesn't handle #includes
		TRACY_ZONEN("Hot reload");

		mReloadCandidates.clear();
		{
			std::lock_guard<std::mutex> lock(mHotReloadMutex);
			mCache.forEach([this](const CachedResource<SourceShader>& entry) {
				const SourceShader& shader = entry.mResource;
				if (shader.mConstructionArgs) {
					mReloadCandidates.push_back({ entry.mHandle, *shader.mConstructionArgs, shader.mModifiedTime, shader.mName });
				}
			});
		}

		for (const ReloadCandidate& candidate : mReloadCandidates) {
			time_t lastModTime = candidate.mModifiedTime;
			for (const std::string& stage : candidate.mConstructionArgs) {
				if (!stage.empty()) {
					lastModTime = std::max(lastModTime, Loader::getFileModTime(stage));
				}
			}
			if (lastModTime > candidate.mModifiedTime) {
				NEO_LOG_I("Hot reloading %s", candidate.mName.c_str());
				// Only queues - the teardown itself happens on the render thread in _tickImpl.
				discard(candidate.mHandle);
			}
		}
	}
}
