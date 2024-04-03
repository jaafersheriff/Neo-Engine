#include "ShaderResourceManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>

namespace neo {
	struct ShaderLoader final : entt::resource_loader<ShaderLoader, SourceShader> {

		std::shared_ptr<SourceShader> load(const ShaderLoadDetails& shaderDetails, const std::string& name) const {
			return std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, SourceShader::ConstructionArgs>) {
					SourceShader::ShaderCode shaderCode;
					for (auto&&[type, filePath] : arg) {
						shaderCode.emplace(type, Loader::loadFileString(filePath));
					}
					auto result = std::make_shared<SourceShader>(name.c_str(), shaderCode);
					result->mConstructionArgs = arg;
					return result;
				}
				else if constexpr (std::is_same_v<T, SourceShader::ShaderCode>) {
					return std::make_shared<SourceShader>(name.c_str(), arg);
				}
				else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, shaderDetails);

			return nullptr;
		}
	};

	ShaderResourceManager::ShaderResourceManager() {
		mFallback = std::make_shared<SourceShader>("Dummy", SourceShader::ShaderCode{
			{ShaderStage::VERTEX, 
				R"(
					void main() {
						gl_Position = vec4(0,0,0,0);
					}
				)"},
			{ShaderStage::FRAGMENT,
				R"(
					out vec4 color;
					void main() {
						color = vec4(0,0,0,0);
					}
				)"}
		});
	}

	ShaderResourceManager::~ShaderResourceManager() {
		mFallback->destroy();
		mFallback.reset();
	}

	const ResolvedShaderInstance& ShaderResourceManager::resolveDefines(ShaderHandle handle, const ShaderDefines& defines) const {
		auto& resolved = resolve(handle).getResolvedInstance(defines);
		resolved.bind();
		return resolved;
	}

	[[nodiscard]] ShaderResourceManager::ShaderHandle ShaderResourceManager::_asyncLoadImpl(ShaderHandle id, ShaderLoadDetails shaderDetails, std::optional<std::string> debugName) const {
		mQueue.emplace_back(ResourceLoadDetails_Internal{ id, shaderDetails, debugName });
		return id;
	}

	void ShaderResourceManager::_tickImpl() {
		TRACY_ZONE();

		std::vector<ResourceLoadDetails_Internal> swapQueue = {};
		std::swap(swapQueue, mQueue);
		mQueue.clear();

		for (auto& loadDetails : swapQueue) {
			mCache.load<ShaderLoader>(loadDetails.mHandle.mHandle, loadDetails.mLoadDetails, loadDetails.mDebugName);
		}
	}

	void ShaderResourceManager::_clearImpl() {
		mQueue.clear();
		mCache.each([](Resource_Internal& resource) {
			resource.mResource.destroy();
		});
		mCache.clear();
	}

	void ShaderResourceManager::imguiEditor() {
		std::optional<entt::id_type> destroyHandle;
		mCache.each([&](entt::id_type enttId, Resource_Internal& resource) {
			auto& shader = resource.mResource;
			if (ImGui::TreeNode(shader.mName.c_str())) {
				if (shader.mConstructionArgs && ImGui::Button("Reload all")) {
					destroyHandle = enttId; // Can't destroy mid-each
					auto newId = asyncLoad(HashedString(shader.mName.c_str()), *shader.mConstructionArgs);
					NEO_UNUSED(newId);
				}

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
		if (destroyHandle) {
			mCache.discard(*destroyHandle);
		}
	}
}