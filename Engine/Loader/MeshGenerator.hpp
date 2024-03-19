#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		// Maybe these shoudl accept the resource manager so they can direclty load
		void generateCube(HashedString, MeshManager&);
		void generateQuad(HashedString, MeshManager&);
		void generateSphere(HashedString, MeshManager&, int recursions);
		Mesh generatePlane(float h, int VERTEX_COUNT, int numOctaves);
	}
}
