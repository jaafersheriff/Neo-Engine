
#include "Model/Mesh.hpp"

namespace neo {

    class MeshGenerator {

    private:
        static Mesh *createMesh(const std::vector<float> &vert, const std::vector<float> &norm, const std::vector<float> &tex, const std::vector<unsigned> &ele) {
            Mesh *mesh = new Mesh;
            mesh->buffers.vertBuf.insert(mesh->buffers.vertBuf.begin(), vert.begin(), vert.end());
            mesh->buffers.norBuf.insert(mesh->buffers.norBuf.begin(), norm.begin(), norm.end());
            mesh->buffers.texBuf.insert(mesh->buffers.texBuf.begin(), tex.begin(), tex.end());
            mesh->buffers.eleBuf.insert(mesh->buffers.eleBuf.begin(), ele.begin(), ele.end());
            mesh->vertBufSize = mesh->buffers.vertBuf.size();
            mesh->norBufSize = mesh->buffers.norBuf.size();
            mesh->texBufSize = mesh->buffers.texBuf.size();
            mesh->eleBufSize = mesh->buffers.eleBuf.size();
            mesh->upload();
            return mesh;
        }

        public:

            static Mesh * createCube() {
                return createMesh(
                    {-0.5f, -0.5f, -0.5f,
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
                     -0.5f,  0.5f,  0.5f },
                    {-0.5f, -0.5f, -0.5f,
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
                     -0.5f,  0.5f,  0.5f },
                    { 0,  0, -1,
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
                      0,  0,  1 },
                    { 0,  1,  2,
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
                     20, 22, 23 } 
                );
            }

            static Mesh * createQuad() {
                return createMesh(
                    {-0.5f, -0.5f,  0.f,
                      0.5f, -0.5f,  0.f,
                     -0.5f,  0.5f,  0.f,
                      0.5f,  0.5f,  0.f },
                    {0.f, 0.f, 1.f,
                     0.f, 0.f, 1.f,
                     0.f, 0.f, 1.f,
                     0.f, 0.f, 1.f
                    }, {}, {});
            }

    };

}