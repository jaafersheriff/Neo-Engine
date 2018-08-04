
#include "Model/Mesh.hpp"

namespace neo {

    class MeshGenerator {

        public:

            static Mesh * createCube() {
                return new Mesh;
            }

            static Mesh * createQuad() {
                return new Mesh;
            }

    };

}