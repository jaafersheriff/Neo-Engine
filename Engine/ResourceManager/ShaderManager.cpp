#include "ShaderManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>
#include <execution>

namespace neo {
	struct ShaderLoader final : entt::resource_loader<ShaderLoader, BackedResource<SourceShader>> {

		std::shared_ptr<BackedResource<SourceShader>> load(const ShaderLoadDetails& shaderDetails, const std::optional<std::string>& debugName) const {
			NEO_ASSERT(debugName.has_value(), "Shaders need to come with a name please");
			NEO_LOG_V("Uploading shader %s", debugName.value().c_str());
			return util::visit(shaderDetails,
				[&](const SourceShader::ConstructionArgs& constructionArgs) {
					SourceShader::ShaderCode shaderCode;
					time_t lastModTime = 0;
					for (auto&& [type, filePath] : constructionArgs) {
						shaderCode.emplace(type, Loader::loadFileString(filePath));
						lastModTime = std::max(lastModTime, Loader::getFileModTime(filePath));
					}
					auto result = std::make_shared<BackedResource<SourceShader>>(debugName->c_str(), shaderCode);
					result->mResource.mConstructionArgs = constructionArgs;
					result->mResource.mModifiedTime = lastModTime;
					return result;
				},
				[&](const SourceShader::ShaderCode& shaderCode) {
					return std::make_shared<BackedResource<SourceShader>>(debugName->c_str(), shaderCode);
				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	};

	ShaderManager::ShaderManager() {
		mFallback = ShaderLoader{}.load(SourceShader::ShaderCode{
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

		mFallback->mResource.getResolvedInstance({});
	}

	ShaderManager::~ShaderManager() {
		mFallback->mResource.destroy();
		mFallback.reset();
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

	[[nodiscard]] ShaderManager::ShaderHandle ShaderManager::_asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, const std::optional<std::string>& debugName) const {
		mQueue.emplace_back(ResourceLoadDetails_Internal{ id, shaderDetails, debugName });
		return id;
	}

	void ShaderManager::_tickImpl() {
		TRACY_ZONE();

		static uint64_t fc = 0;
		fc++;
		if (fc % 10 == 0) {
			TRACY_ZONEN("Shader Hot Reload");
			// Cache doesn't support iterators :/ 
			std::vector<entt::id_type> list;
			{
				TRACY_ZONEN("Prealloc");
				list.reserve(mCache.size());
				mCache.each([&list](const entt::id_type enttId) {
					list.emplace_back(enttId);
					});
			}

			{
				TRACY_ZONEN("Parallel");
				std::for_each(std::execution::par, list.begin(), list.end(), [this](const entt::id_type& id) {
					if (auto resource = mCache.handle(id)) {
						tracy::SetThreadName(resource->mResource.mName.c_str());
						if (resource->mResource.mConstructionArgs) {
							time_t lastModTime = resource->mResource.mModifiedTime;
							for (auto&& [stage, fileName] : *resource->mResource.mConstructionArgs) {
								lastModTime = std::max(lastModTime, Loader::getFileModTime(fileName));
							}
							if (lastModTime > resource->mResource.mModifiedTime) {
								NEO_LOG_I("Hot reloading %s", resource->mResource.mName.c_str());
								ShaderHandle handle(id);

								// This should really have a mutex on it, but how are you gunna be editing >1 file at a time come on now
								discard(handle);
							}
						}
					}
					});
			}
		}

		{
			std::vector<ResourceLoadDetails_Internal> swapQueue = {};
			std::swap(swapQueue, mQueue);
			mQueue.clear();

			for (auto& loadDetails : swapQueue) {
				mCache.load<ShaderLoader>(loadDetails.mHandle.mHandle, loadDetails.mLoadDetails, loadDetails.mDebugName);
			}
		}

		{
			std::vector<ShaderHandle> swapQueue = {};
			std::swap(swapQueue, mDiscardQueue);
			mDiscardQueue.clear();
			for (auto& id : swapQueue) {
				if (isValid(id)) {
					_destroyImpl(mCache.handle(id.mHandle).get());
					mCache.discard(id.mHandle);
				}
			}
		}
	}

	void ShaderManager::_destroyImpl(BackedResource<SourceShader>& sourceShader) {
		if (sourceShader.mResource.mConstructionArgs.has_value()) {
			for (auto&& [type, charString] : sourceShader.mResource.mShaderSources) {
				delete charString;
			}
		}
		sourceShader.mResource.destroy();
	}

	void ShaderManager::imguiEditor() {
		mCache.each([&](entt::id_type, BackedResource<SourceShader>& resource) {
			auto& shader = resource.mResource;
			if (ImGui::TreeNode(shader.mName.c_str())) {
				if (shader.mResolvedShaders.size()) {
					if (ImGui::TreeNode("##idk", "Variants (%d)", static_cast<int>(shader.mResolvedShaders.size()))) {
						ImGui::Separator();
						for (const auto& variant : shader.mResolvedShaders) {
							// if (mConstructionArgs && ImGui::Button("Reload")) {
							// Just destroy the variant and evict from the map, easy
							// }
							// else {
							ImGui::Text("%s", variant.second.variant().size() ? variant.second.variant().c_str() : "No defines");
							ImGui::Separator();
							// }
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
}