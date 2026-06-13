#include "Renderer/pch.hpp"
#include "ShaderBarrier.hpp"

#include "GL/glew.h"

namespace neo {
	namespace {
		int32_t _getGLBarrierType(types::shader::Barrier barrierType) {
			switch (barrierType) {
			// case types::shader::Barrier::AtomicCounter:
			// 	return GL_ATOMIC_COUNTER_BARRIER_BIT;
			case types::shader::Barrier::ImageAccess:
				return GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
			case types::shader::Barrier::StorageBuffer:
				return GL_SHADER_STORAGE_BARRIER_BIT;
			default:
				NEO_FAIL("Invalid barrier type");
				return 0;
			}
		}
	}
	ShaderBarrier::ShaderBarrier(types::shader::Barrier barrierType) :
			mBarrierType(barrierType)
		{}

	ShaderBarrier::~ShaderBarrier() {
		if (mBarrierType != types::shader::Barrier::None) {
			glMemoryBarrier(_getGLBarrierType(mBarrierType));
		}
	}
}
