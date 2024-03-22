#include "ShaderResourceManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"

#include <imgui.h>

namespace neo {
	struct ShaderLoader final : entt::resource_loader<ShaderLoader, SourceShader> {

		std::shared_ptr<SourceShader> load(const std::string& name, const ShaderResourceManager::ShaderLoadDetails& shaderDetails, std::shared_ptr<SourceShader> fallback) const {
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
					return fallback;
				}
			}, shaderDetails);
		}
	};

	ShaderResourceManager::ShaderResourceManager() {
		mDummyShader = std::make_shared<SourceShader>("Dummy", SourceShader::ShaderCode{
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
		mDummyShader->destroy();
		mDummyShader.reset();
	}

	bool ShaderResourceManager::isValid(ShaderHandle id) const {
		return mShaderCache.contains(id);
	}

	const ResolvedShaderInstance& ShaderResourceManager::get(HashedString id, const ShaderDefines& defines) const {
		return get(id.value(), defines);
	}

	const ResolvedShaderInstance& ShaderResourceManager::get(ShaderHandle handle, const ShaderDefines& defines) const {
		if (isValid(handle)) {
			const auto& resolvedInstance = mShaderCache.handle(handle).get().getResolvedInstance(defines);
			if (resolvedInstance.isValid()) {
				resolvedInstance.bind();
				return resolvedInstance;
			}
		}
		else {
			NEO_FAIL("Invalid shader requested, did you check for validity?");
		}
		auto& dummy = mDummyShader->getResolvedInstance({});
		dummy.bind();
		return dummy;
	}

	[[nodiscard]] ShaderHandle ShaderResourceManager::asyncLoad(const char* name, ShaderLoadDetails shaderDetails) const {
		HashedString id(name);
		if (!isValid(id) && mQueue.find(name) == mQueue.end()) {
			mQueue.emplace(std::string(name), shaderDetails);
		}
		return id;
	}

	void ShaderResourceManager::_tick() {
		TRACY_ZONE();


		std::map<std::string, ShaderLoadDetails> swapQueue = {};
		std::swap(swapQueue, mQueue);
		mQueue.clear();

		for (auto&& [name, shaderDetails] : swapQueue) {
			mShaderCache.load<ShaderLoader>(HashedString(name.c_str()).value(), name, shaderDetails, mDummyShader);
		}
	}

	void ShaderResourceManager::clear() {
		mQueue.clear();
		mShaderCache.each([](SourceShader& shader) {
			shader.destroy();
		});
		mShaderCache.clear();
	}

	void ShaderResourceManager::imguiEditor() {
		std::optional<ShaderHandle> destroyHandle;
		if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
			mShaderCache.each([&](const ShaderHandle handle, SourceShader& shader) {
				if (!isValid(handle)) {
					return;
				}
				if (ImGui::TreeNode(shader.mName.c_str())) {
					if (shader.mConstructionArgs && ImGui::Button("Reload all")) {
						destroyHandle = handle; // Can't destroy mid-each
						NEO_UNUSED(asyncLoad(shader.mName.c_str(), *shader.mConstructionArgs));
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
			ImGui::TreePop();
		}
		if (destroyHandle) {
			mShaderCache.discard(*destroyHandle);
		}
	}
}