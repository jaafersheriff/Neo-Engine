
#include "GLHelper/Mesh.hpp"

#include "Util/Util.hpp"

namespace neo {

    class MeshGenerator {

    private:
        static Mesh *createMesh(const std::vector<float> &vert, const std::vector<float> &norm, const std::vector<float> &tex, const std::vector<unsigned> &ele, unsigned mode = GL_TRIANGLES) {
            Mesh *mesh = new Mesh;
            mesh->buffers.vertBuf.insert(mesh->buffers.vertBuf.begin(), vert.begin(), vert.end());
            mesh->buffers.norBuf.insert(mesh->buffers.norBuf.begin(), norm.begin(), norm.end());
            mesh->buffers.texBuf.insert(mesh->buffers.texBuf.begin(), tex.begin(), tex.end());
            mesh->buffers.eleBuf.insert(mesh->buffers.eleBuf.begin(), ele.begin(), ele.end());
            mesh->vertBufSize = mesh->buffers.vertBuf.size();
            mesh->norBufSize = mesh->buffers.norBuf.size();
            mesh->texBufSize = mesh->buffers.texBuf.size();
            mesh->eleBufSize = mesh->buffers.eleBuf.size();
            mesh->upload(mode);
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
                     0.f, 0.f, 1.f }, 
                    {0.f, 0.f, 
                     1.f, 0.f,
                     0.f, 1.f,
                     1.f, 1.f }, {}, GL_TRIANGLE_STRIP);
            }

            // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
            static Mesh * createSphere(int recursions) {
	            float t = (float) (1.f + (glm::sqrt(5.0)) / 2.f);
                float length = glm::length(glm::vec3(1, 0, t));
                std::vector<float> verts = {
                     -1.f / length,    t / length,  0.f / length,
                      1.f / length,    t / length,  0.f / length,
                     -1.f / length,   -t / length,  0.f / length,
                      1.f / length,   -t / length,  0.f / length,
                      0.f / length, -1.f / length,    t / length,
                      0.f / length,  1.f / length,    t / length,
                      0.f / length, -1.f / length,   -t / length,
                      0.f / length,  1.f / length,   -t / length,
                        t / length,  0.f / length, -1.f / length,
                        t / length,  0.f / length,  1.f / length,
                       -t / length,  0.f / length, -1.f / length,
                       -t / length,  0.f / length,  1.f / length
                };

                std::vector<unsigned> ele = {
                      0, 11,  5,
                      0,  5,  1,
                      0,  1,  7,
                      0,  7, 10,
                      0, 10, 11,
                      1,  5,  9,
                      5, 11,  4,
                     11, 10,  2,
                     10,  7,  6,
                      7,  1,  8,
                      3,  9,  4,
                      3,  4,  2,
                      3,  2,  6,
                      3,  6,  8,
                      3,  8,  9,
                      4,  9,  5,
                      2,  4, 11,
                      6,  2, 10,
                      8,  6,  7,
                      9,  8,  1,
                };

                for (unsigned i = 1; i <= recursions; i++) {
                    std::vector<unsigned> ele2;
                    for (unsigned j = 0; j <= ele.size() - 3; j += 3) {
                        // find 3 verts of old face
                        glm::vec3 v1(verts[3 * ele[j + 0] + 0], verts[3 * ele[j + 0] + 1], verts[3 * ele[j + 0] + 2]);
                        glm::vec3 v2(verts[3 * ele[j + 1] + 0], verts[3 * ele[j + 1] + 1], verts[3 * ele[j + 1] + 2]);
                        glm::vec3 v3(verts[3 * ele[j + 2] + 0], verts[3 * ele[j + 2] + 1], verts[3 * ele[j + 2] + 2]);

                        // add verts of new tris
                        glm::vec3 halfA = glm::normalize((v1 + v2) / 2.f);
                        glm::vec3 halfB = glm::normalize((v2 + v3) / 2.f);
                        glm::vec3 halfC = glm::normalize((v3 + v1) / 2.f);
                        verts.push_back(halfA.x);
                        verts.push_back(halfA.y);
                        verts.push_back(halfA.z);
                        verts.push_back(halfB.x);
                        verts.push_back(halfB.y);
                        verts.push_back(halfB.z);
                        verts.push_back(halfC.x);
                        verts.push_back(halfC.y);
                        verts.push_back(halfC.z);

                        // add indices of new faces 
                        int indA = verts.size() / 3 - 3;
                        int indB = verts.size() / 3 - 2;
                        int indC = verts.size() / 3 - 1;
                        ele2.push_back(ele[j + 0]);
                        ele2.push_back(indA);
                        ele2.push_back(indC);
                        ele2.push_back(ele[j + 1]);
                        ele2.push_back(indB);
                        ele2.push_back(indA);
                        ele2.push_back(ele[j + 2]);
                        ele2.push_back(indC);
                        ele2.push_back(indB);
                        ele2.push_back(indA);
                        ele2.push_back(indB);
                        ele2.push_back(indC);
                    }

                    ele = ele2;
                }

                // calculate UV coords
                std::vector<float> tex;
                for (int i = 0; i < verts.size(); i += 3) {
                    tex.push_back(glm::clamp(0.5f + std::atan2(verts[i + 2], verts[i]) / (2.f * Util::PI()), 0.f, 1.f));
                    tex.push_back(glm::clamp(0.5f + std::asin(verts[i+1]) / Util::PI(), 0.f, 1.f));
                }

                return createMesh(
                    verts,
                    verts,
                    tex,
                    ele
                );
            }
    };

}