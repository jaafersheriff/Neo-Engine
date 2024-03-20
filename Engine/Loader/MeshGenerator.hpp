#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		// Maybe these shoudl accept the resource manager so they can direclty load
		void generateCube(HashedString, MeshResourceManager&);
		void generateQuad(HashedString, MeshResourceManager&);
		void generateSphere(HashedString, MeshResourceManager&, int recursions);
	}
}
