
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

            // From https://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c/5989676#5989676
            static Mesh * createSphere(float radius = 1.f, int rings = 30, int sectors = 30) {
                std::vector<float> vertices, normals, texcoords;
                std::vector<unsigned> indices;

                float const R = 1.f/(float)(rings-1);
                float const S = 1.f/(float)(sectors-1);

                for(int r = 0; r < rings; ++r) {
                    for(int s = 0; s < sectors; ++s) {
                        texcoords.push_back(s*S);
                        texcoords.push_back(r*R);

                        glm::vec3 vert = glm::normalize(glm::vec3(
                            glm::cos(2*Util::PI() * s * S) * glm::sin( Util::PI() * r * R ),
                            glm::sin( -Util::PI()/2 + Util::PI() * r * R ),
                            glm::sin(2*Util::PI() * s * S) * glm::sin( Util::PI() * r * R )));

                        vertices.push_back(vert.x * radius);
                        vertices.push_back(vert.y * radius);
                        vertices.push_back(vert.z * radius);
                        normals.push_back(vert.x);
                        normals.push_back(vert.y);
                        normals.push_back(vert.z);

                        int curRow = r * sectors;
                        int nextRow = (r+1) * sectors;

                        indices.push_back(curRow + s);
                        indices.push_back(nextRow + s);
                        indices.push_back(nextRow + (s+1));

                        indices.push_back(curRow + s);
                        indices.push_back(nextRow + (s+1));
                        indices.push_back(curRow + (s+1));
                    }
                }

                return createMesh(vertices, normals, texcoords, indices);
 
            }
    };

}