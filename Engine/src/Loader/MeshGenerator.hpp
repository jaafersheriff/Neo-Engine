
#include "Model/Mesh.hpp"

namespace neo {

    class MeshGenerator {

        public:

            static Mesh * createCube() {
                Mesh *mesh = new Mesh;
                mesh->buffers.vertBuf = {
                    -0.5f, -0.5f, -0.5f,
                     0.5f,  0.5f, -0.5f,
                     0.5f, -0.5f, -0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f, -0.5f,
                    -0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
                     0.5f,  0.5f,  0.5f,
                     0.5f,  0.5f, -0.5f,
                    -0.5f,  0.5f,  0.5f,
                     0.5f, -0.5f, -0.5f,
                     0.5f,  0.5f, -0.5f,
                     0.5f,  0.5f,  0.5f,
                     0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f, -0.5f,
                     0.5f, -0.5f, -0.5f,
                     0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                     0.5f, -0.5f,  0.5f,
                     0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f,  0.5f
                };
                mesh->buffers.norBuf = {
                     0,  0, -1,
                     0,  0, -1,
                     0,  0, -1,
                     0,  0, -1,
                    -1,  0,  0,
                    -1,  0,  0,
                    -1,  0,  0,
                    -1,  0,  0,
                     0,  1,  0,
                     0,  1,  0,
                     0,  1,  0,
                     0,  1,  0,
                     1,  0,  0,
                     1,  0,  0,
                     1,  0,  0,
                     1,  0,  0,
                     0, -1,  0,
                     0, -1,  0,
                     0, -1,  0,
                     0, -1,  0,
                     0,  0,  1,
                     0,  0,  1,
                     0,  0,  1,
                     0,  0,  1,
                };
                mesh->buffers.eleBuf = {
                     0,  1,  2,
                     0,  3,  1,
                     4,  5,  6,
                     4,  7,  5,
                     8,  9, 10,
                     8, 11,  9,
                    12, 13, 14,
                    12, 14, 15,
                    16, 17, 18,
                    16, 18, 19,
                    20, 21, 22,
                    20, 22, 23,
                };
                mesh->buffers.texBuf = {
                    -0.5f, -0.5f, -0.5f,
                     0.5f,  0.5f, -0.5f,
                     0.5f, -0.5f, -0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f, -0.5f,
                    -0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f,  0.5f, -0.5f,
                     0.5f,  0.5f,  0.5f,
                     0.5f,  0.5f, -0.5f,
                    -0.5f,  0.5f,  0.5f,
                     0.5f, -0.5f, -0.5f,
                     0.5f,  0.5f, -0.5f,
                     0.5f,  0.5f,  0.5f,
                     0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f, -0.5f,
                     0.5f, -0.5f, -0.5f,
                     0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                    -0.5f, -0.5f,  0.5f,
                     0.5f, -0.5f,  0.5f,
                     0.5f,  0.5f,  0.5f,
                    -0.5f,  0.5f,  0.5f
                };
                mesh->vertBufSize = int(mesh->buffers.vertBuf.size());
                mesh->norBufSize = int(mesh->buffers.norBuf.size());
                mesh->texBufSize = int(mesh->buffers.texBuf.size());
                mesh->eleBufSize = int(mesh->buffers.eleBuf.size());
                mesh->upload();
                return mesh;
            }

            static Mesh * createQuad() {
                Mesh *mesh = new Mesh;
                mesh->buffers.vertBuf = {
                    -0.5f, -0.5f,  0.f,
                     0.5f, -0.5f,  0.f,
                    -0.5f,  0.5f,  0.f,
                     0.5f,  0.5f,  0.f
                };
                mesh->vertBufSize = mesh->buffers.vertBuf.size();
                mesh->buffers.norBuf = {
                    0.f, 0.f, 1.f,
                    0.f, 0.f, 1.f,
                    0.f, 0.f, 1.f,
                    0.f, 0.f, 1.f
                };
                mesh->norBufSize = mesh->buffers.norBuf.size();
                mesh->upload();
                return mesh;
            }

    };

}