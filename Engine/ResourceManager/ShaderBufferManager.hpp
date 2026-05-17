#pragma once

#include "ResourceManagerInterface.hpp"

#include "Renderer/GLObjects/ShaderBuffer.hpp"
#include "Renderer/GLObjects/ShaderBarrier.hpp"

namespace neo {
	class ResourceManagers;

	struct ShaderBufferLoadDetails {
		types::buffer::Target mTarget = types::buffer::Target::ShaderStorage;
		uint32_t mByteSize = 0;
		const uint8_t* mData = nullptr;
	};
	using ShaderBufferHandle = ResourceHandle<ShaderBuffer>;

	class ShaderBufferManager final : public ResourceManagerInterface<ShaderBufferManager, ShaderBuffer, ShaderBufferLoadDetails> {
		friend ResourceManagerInterface;
	public:

		ShaderBufferManager();
		~ShaderBufferManager();

		void imguiEditor();

	protected:
		[[nodiscard]] ShaderBufferHandle _asyncLoadImpl(ShaderBufferHandle id, ShaderBufferLoadDetails details, const std::optional<std::string>& debugName) const;
		void _destroyImpl(BackedResource<ShaderBuffer>& buffer);
		void _tickImpl();
	};
}
