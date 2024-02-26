#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		Mesh* generateCube();
		Mesh* generateQuad();
		Mesh* generateSphere(int recursions);
		Mesh* generatePlane(float h, int VERTEX_COUNT, int numOctaves);
	}
}
