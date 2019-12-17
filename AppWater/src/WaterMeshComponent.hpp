#pragma once

#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    class WaterMeshComponent : public MeshComponent {

    public:

        std::vector<float> vertices;
        std::vector<float> texCoords;

        glm::vec3 tessFactor = glm::vec3(10.f, 7.f, 3.f);
        glm::vec2 tessDistance = glm::vec2(2.f, 5.f);

        WaterMeshComponent(GameObject *go, int xTiles, int zTiles, float texTileX, float texTileZ) :
            MeshComponent(go, new Mesh(GL_PATCHES))
        {
            uint32_t vertexCount = xTiles * zTiles * 6;

            float oneOverXTiles = 1.0f / static_cast<float>(xTiles);
            float oneOverZTiles = 1.0f / static_cast<float>(zTiles);
            vertices.resize(vertexCount * 3);
            texCoords.resize(vertexCount * 4);

            for (uint32_t x = 0; x < xTiles; x++)
            {
                for (uint32_t z = 0; z < zTiles; z++)
                {
                    uint32_t tileIndex = (x * zTiles + z) * 6;
                    float xBeginTile = (oneOverXTiles * static_cast<float>(x)) * texTileX;
                    float xEndTile = (oneOverXTiles * static_cast<float>((x + 1))) * texTileX;
                    float zBeginTile = (oneOverZTiles * static_cast<float>(z)) * texTileZ;
                    float zEndTile = (oneOverZTiles * static_cast<float>((z + 1))) * texTileZ;

                    float xBegin = (oneOverXTiles * static_cast<float>(x));
                    float xEnd = (oneOverXTiles * static_cast<float>((x + 1)));
                    float zBegin = (oneOverZTiles * static_cast<float>(z));
                    float zEnd = (oneOverZTiles * static_cast<float>((z + 1)));

                    vertices[3 * tileIndex + 0] = static_cast<float>(x);
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z);
                    texCoords[4 * tileIndex + 0] = xBeginTile;
                    texCoords[4 * tileIndex + 1] = zBeginTile;
                    texCoords[4 * tileIndex + 2] = xBegin;
                    texCoords[4 * tileIndex + 3] = zBegin;
                    tileIndex++;

                    vertices[3 * tileIndex + 0] = static_cast<float>(x);
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[4 * tileIndex + 0] = xBeginTile;
                    texCoords[4 * tileIndex + 1] = zEndTile;
                    texCoords[4 * tileIndex + 2] = xBegin;
                    texCoords[4 * tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[3 * tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z);
                    texCoords[4 * tileIndex + 0] = xEndTile;
                    texCoords[4 * tileIndex + 1] = zBeginTile;
                    texCoords[4 * tileIndex + 2] = xEnd;
                    texCoords[4 * tileIndex + 3] = zBegin;
                    tileIndex++;

                    vertices[3 * tileIndex + 0] = static_cast<float>(x);
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[4 * tileIndex + 0] = xBeginTile;
                    texCoords[4 * tileIndex + 1] = zEndTile;
                    texCoords[4 * tileIndex + 2] = xBegin;
                    texCoords[4 * tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[3 * tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[4 * tileIndex + 0] = xEndTile;
                    texCoords[4 * tileIndex + 1] = zEndTile;
                    texCoords[4 * tileIndex + 2] = xEnd;
                    texCoords[4 * tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[3 * tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[3 * tileIndex + 1] = 0.f;
                    vertices[3 * tileIndex + 2] = static_cast<float>(z);
                    texCoords[4 * tileIndex + 0] = xEndTile;
                    texCoords[4 * tileIndex + 1] = zBeginTile;
                    texCoords[4 * tileIndex + 2] = xEnd;
                    texCoords[4 * tileIndex + 3] = zBegin;
                    tileIndex++;
                }
            }
        }

        virtual void init() override {
            mMesh->addVertexBuffer(VertexType::Position, 0, 3, vertices);
            mMesh->addVertexBuffer(VertexType::Texture0, 1, 4, texCoords);
        }

        virtual void imGuiEditor() override {
            ImGui::SliderFloat3("TessFactor", &tessFactor[0], 0.1f, 10.f);
            ImGui::SliderFloat2("TessDistance", &tessDistance[0], 0.1f, 30.f);
        }

    };
}