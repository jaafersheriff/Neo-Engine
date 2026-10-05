#include "GLBackend/pch.hpp"
#include "GLDevice.hpp"

#include "GLBackend/GLHelper.hpp"

#include "Jobs/JobThread.hpp"
#include "Util/Assert.hpp"

#include <GL/glew.h>
#include <tracy/TracyOpenGL.hpp>

namespace neo {

	namespace GLDevice {

		void init(RendererDetails& details) {
			NEO_ASSERT(isRenderThread(), "GLDevice initialized off the render thread - the GL context belongs to it");

			glewExperimental = GL_FALSE;
			NEO_ASSERT(glewInit() == GLEW_OK, "Failed to init GLEW");

			// Tracy's GPU context is per-thread and must be created on the thread that issues the queries.
			TracyGpuContext;

	#ifdef DEBUG_MODE
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(GLHelper::OpenGLMessageCallback, nullptr);

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
			glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif
			/* Set max work group */
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &details.mMaxComputeWorkGroupSize.x);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &details.mMaxComputeWorkGroupSize.y);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &details.mMaxComputeWorkGroupSize.z);
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &details.mMaxTextureArrayLayers);
			details.mVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
			details.mRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
			details.mShadingLanguage = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		}

		void applyDefaultState() {
			NEO_ASSERT(isRenderThread(), "GL state set off the render thread - the GL context belongs to it");

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
	}

}
