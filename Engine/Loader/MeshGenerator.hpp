#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		// Maybe these shoudl accept the resource manager so they can direclty load
		Mesh generateCube();
		Mesh generateQuad();
		Mesh generateSphere(int recursions);
		Mesh generatePlane(float h, int VERTEX_COUNT, int numOctaves);
	}
}
