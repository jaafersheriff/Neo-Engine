
#include "GLObjects/Mesh.hpp"

#include "ext/PerlinNoise.hpp"

namespace neo {

    class MeshGenerator {

        public:

            static void generateCube(Mesh* mesh) {
                std::vector<float> verts =
                { -0.5f, -0.5f, -0.5f,
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
                 -0.5f,  0.5f,  0.5f };
                mesh->addVertexBuffer(VertexType::Position, 0, 3, verts);
                mesh->mMin = glm::vec3(-0.5f);
                mesh->mMax = glm::vec3( 0.5f);

                std::vector<float> normals =
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
                  0,  0,  1 };
                mesh->addVertexBuffer(VertexType::Normal, 1, 3, normals);

                std::vector<float> uvs =
                { 1.f, 0.f,
                    0.f, 1.f,
                    0.f, 0.f,
                    1.f, 1.f,

                    0.f, 0.f,
                    1.f, 1.f,
                    0.f, 1.f,
                    1.f, 0.f,

                    1.f, 0.f,
                    0.f, 1.f,
                    0.f, 0.f,
                    1.f, 1.f,

                    1.f, 0.f,
                    1.f, 1.f,
                    0.f, 1.f,
                    0.f, 0.f,

                    1.f, 1.f,
                    0.f, 1.f,
                    0.f, 0.f,
                    1.f, 0.f,

                    0.f, 0.f,
                    1.f, 0.f,
                    1.f, 1.f,
                    0.f, 1.f };
                mesh->addVertexBuffer(VertexType::Texture0, 2, 2, uvs);

                std::vector<unsigned> indices =
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
                 20, 22, 23 };
                mesh->addElementBuffer(indices);

                mesh->mPrimitiveType = GL_TRIANGLES;
            }

            static void generateQuad(Mesh* mesh) {
                std::vector<float> verts =
                { -0.5f, -0.5f,  0.f,
                   0.5f, -0.5f,  0.f,
                  -0.5f,  0.5f,  0.f,
                   0.5f,  0.5f,  0.f };
                mesh->addVertexBuffer(VertexType::Position, 0, 3, verts);
                mesh->mMin = glm::vec3(-0.5f, -0.5f, -0.1f);
                mesh->mMax = glm::vec3( 0.5f,  0.5f,  0.1f);

                std::vector<float> normals =
                { 0.f, 0.f, 1.f,
                  0.f, 0.f, 1.f,
                  0.f, 0.f, 1.f,
                  0.f, 0.f, 1.f };
                mesh->addVertexBuffer(VertexType::Normal, 1, 3, normals);

                std::vector<float> uvs =
                { 0.f, 0.f,
                  1.f, 0.f,
                  0.f, 1.f,
                  1.f, 1.f };
                mesh->addVertexBuffer(VertexType::Texture0, 2, 2, uvs);

                std::vector<unsigned> indices =
                { 0, 1, 2,
                  1, 3, 2 };
                mesh->addElementBuffer(indices);

                mesh->mPrimitiveType = GL_TRIANGLES;
            }

            // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
            static void generateSphere(Mesh* mesh, int recursions) {
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
                mesh->mMin = glm::vec3(-t / length);
                mesh->mMax = glm::vec3( t / length);

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

                for (int i = 1; i <= recursions; i++) {
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
                        mesh->mMin = glm::min(mesh->mMin, glm::min(halfA, glm::min(halfB, halfC)));
                        mesh->mMax = glm::max(mesh->mMax, glm::max(halfA, glm::max(halfB, halfC)));

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
                for (unsigned i = 0; i < verts.size(); i += 3) {
                    tex.push_back(glm::clamp(0.5f + std::atan2(verts[i + 2], verts[i]) / (2.f * Util::PI()), 0.f, 1.f));
                    tex.push_back(glm::clamp(0.5f + std::asin(verts[i+1]) / Util::PI(), 0.f, 1.f));
                }

                mesh->addVertexBuffer(VertexType::Position, 0, 3, verts);
                mesh->addVertexBuffer(VertexType::Normal, 1, 3, verts);
                mesh->addVertexBuffer(VertexType::Texture0, 2, 2, tex);
                mesh->addElementBuffer(ele);

                mesh->mPrimitiveType = GL_TRIANGLES;
            }

            static void generatePlane(Mesh* mesh, float h, int VERTEX_COUNT, int numOctaves) {
                siv::PerlinNoise noise(rand());

                mesh->mMin = glm::vec3(FLT_MAX);
                mesh->mMax = glm::vec3(FLT_MIN);

                int count = VERTEX_COUNT * VERTEX_COUNT;

                std::vector<std::vector<float>> heights;
                heights.resize(VERTEX_COUNT);
                for (int i = 0; i < VERTEX_COUNT; i++) {
                    heights[i].resize(VERTEX_COUNT);
                }

                std::vector<float> vertices;
                vertices.resize(count * 3);

                std::vector<float> normals;
                normals.resize(count * 3);

                std::vector<float> textureCoords;
                textureCoords.resize(count * 2);

                std::vector<unsigned> indices;
                indices.resize(6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT * 1));

                for (int i = 0; i < VERTEX_COUNT; i++) {
                    for (int j = 0; j < VERTEX_COUNT; j++) {
                        heights[j][i] = h * noise.octaveNoise(j / static_cast<double>(VERTEX_COUNT), i / static_cast<double>(VERTEX_COUNT), numOctaves);
                    }
                }

                int vertexPointer = 0;
                for (int i = 0; i < VERTEX_COUNT; i++) {
                    for (int j = 0; j < VERTEX_COUNT; j++) {

                        glm::vec3 vert = glm::vec3(
                            (float)j / ((float)VERTEX_COUNT - 1),
                            heights[j][i],
                            (float)i / ((float)VERTEX_COUNT - 1));

                        vertices[vertexPointer * 3 + 0] = vert.x;
                        vertices[vertexPointer * 3 + 1] = vert.y;
                        vertices[vertexPointer * 3 + 2] = vert.z;

                        float heightL = j == 0                ? 0.f : heights[j - 1][i    ] / h;
                        float heightR = j == VERTEX_COUNT - 1 ? 0.f : heights[j + 1][i    ] / h;
                        float heightD = i == VERTEX_COUNT - 1 ? 0.f : heights[j    ][i + 1] / h;
                        float heightU = i == 0                ? 0.f : heights[j    ][i - 1] / h;
                        glm::vec3 normal = glm::normalize(glm::vec3(heightL - heightR, 2.f / ((float)VERTEX_COUNT - 1), heightD - heightU));
                        normals[vertexPointer * 3 + 0] = normal.x;
                        normals[vertexPointer * 3 + 1] = normal.y;
                        normals[vertexPointer * 3 + 2] = normal.z;

                        textureCoords[vertexPointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
                        textureCoords[vertexPointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);


                        mesh->mMin = glm::min(mesh->mMin, vert);
                        mesh->mMax = glm::max(mesh->mMax, vert);
                        vertexPointer++;
                    }
                }

                int indexPointer = 0;
                for (int i = 0; i < VERTEX_COUNT - 1; i++) {
                    for (int j = 0; j < VERTEX_COUNT - 1; j++) {

                        int topLeft = (i*VERTEX_COUNT) + j;
                        int topRight = topLeft + 1;
                        int bottomLeft = ((i + 1)*VERTEX_COUNT) + j;
                        int bottomRight = bottomLeft + 1;
                        indices[indexPointer++] = topLeft;
                        indices[indexPointer++] = bottomLeft;
                        indices[indexPointer++] = topRight;
                        indices[indexPointer++] = topRight;
                        indices[indexPointer++] = bottomLeft;
                        indices[indexPointer++] = bottomRight;
                    }
                }

                mesh->addVertexBuffer(VertexType::Position, 0, 3, vertices);
                mesh->addVertexBuffer(VertexType::Normal, 1, 3, normals);
                mesh->addVertexBuffer(VertexType::Texture0, 2, 2, textureCoords);
                mesh->addElementBuffer(indices);

                mesh->mPrimitiveType = GL_TRIANGLES;
            }
    };

}
