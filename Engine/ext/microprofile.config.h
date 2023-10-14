#define MICROPROFILE_GPU_TIMERS_GL 0
#define MICROPROFILE_ENABLED 0
#define MICROPROFILE_GPU_TIMERS 0

#ifdef MICROPROFILE_IMPL
	#ifndef GLEW_STATIC
		#define GLEW_STATIC
	#endif

	#include <GL/glew.h>
#endif
