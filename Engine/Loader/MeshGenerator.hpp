#pragma once

#include "Renderer/GLObjects/Mesh.hpp"
#include "ext/PerlinNoise.hpp"

namespace neo {

	namespace prefabs {

		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateCube();
		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateQuad();
		[[nodiscard]] std::unique_ptr<MeshLoadDetails> generateSphere(int recursions);
	}
}
