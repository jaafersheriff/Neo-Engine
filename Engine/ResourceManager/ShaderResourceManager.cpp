#include "ShaderResourceManager.hpp"

#include "Loader/Loader.hpp"

#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Util/Profiler.hpp"

namespace neo {

	struct ShaderLoader final : entt::resource_loader<ShaderLoader, SourceShader> {

		std::shared_ptr<SourceShader> load(std::string& name, ShaderResourceManager::ShaderBuilder shaderDetails, std::shared_ptr<SourceShader> fallback) const {
			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, SourceShader::ConstructionArgs>) {
					SourceShader::ShaderCode shaderCode;
					for (auto&&[type, filePath] : arg) {
						shaderCode.emplace(type, Loader::loadFileString(filePath));
					}
					auto result = std::make_shared<SourceShader>(name.c_str(), shaderCode);
					result->mConstructionArgs = arg;
					for (auto&&[type, fileString] : shaderCode) {
						free(const_cast<char*>(fileString));
					}
					return result;
				}
				else if constexpr (std::is_same_v<T, SourceShader::ShaderCode>) {
					return std::make_shared<SourceShader>(name.c_str(), arg);
				}
				else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, shaderDetails);
			return fallback;
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
		NEO_LOG_E("Invalid shader requested");
		return mDummyShader->getResolvedInstance({});
	}

	[[nodiscard]] ShaderHandle ShaderResourceManager::asyncLoad(HashedString id, ShaderBuilder shaderDetails) const {
		mQueue.emplace_back(std::make_pair( std::string(id.data()), shaderDetails));
		return id;
	}

	void ShaderResourceManager::_tick() {
		TRACY_ZONE();
		for (auto&& [name, shaderDetails] : mQueue) {
			mShaderCache.load<ShaderLoader>(HashedString(name.c_str()), name, shaderDetails, mDummyShader);
		}
	}

	void ShaderResourceManager::clear() {
		mShaderCache.each([](SourceShader& shader) {
			shader.destroy();
		});
		mShaderCache.clear();
	}
}