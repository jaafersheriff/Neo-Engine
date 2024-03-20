#include "ShaderResourceManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	struct ShaderLoader final : entt::resource_loader<ShaderLoader, SourceShader> {

		std::shared_ptr<SourceShader> load(std::string& name, ShaderResourceManager::ShaderBuilder shaderDetails, std::shared_ptr<SourceShader> fallback) const {
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

	bool ShaderResourceManager::isValid(ShaderHandle id) const {
		return mShaderCache.contains(id);
	}

	const ResolvedShaderInstance& ShaderResourceManager::get(HashedString id, const ShaderDefines& defines) const {
		return get(id.value(), defines);
	}

	const ResolvedShaderInstance& ShaderResourceManager::get(ShaderHandle id, const ShaderDefines& defines) const {
		if (mShaderCache.contains(id)) {
			const auto& resolvedInstance = mShaderCache.handle(id).get().getResolvedInstance(defines);
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

	[[nodiscard]] ShaderHandle ShaderResourceManager::asyncLoad(HashedString id, ShaderBuilder shaderDetails) const {
		if (!isValid(id)) {
			mQueue.emplace_back(std::make_pair(std::string(id.data()), shaderDetails));
		}
		return id;
	}

	void ShaderResourceManager::_tick() {
		TRACY_ZONE();
		for (auto&& [name, shaderDetails] : mQueue) {
			mShaderCache.load<ShaderLoader>(HashedString(name.c_str()), name, shaderDetails, mDummyShader);
		}
		mQueue.clear();
	}

	void ShaderResourceManager::clear() {
		mShaderCache.each([](SourceShader& shader) {
			shader.destroy();
		});
		mShaderCache.clear();
	}
}