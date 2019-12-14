#pragma once

#include "Component/ModelComponent/MeshComponent.hpp"

#include "GLObjects/GLHelper.hpp"

namespace neo {

    class WaterMeshComponent : public MeshComponent {

    public:

        std::vector<float> vertices;
        std::vector<float> texCoords;

        WaterMeshComponent(GameObject *go, int xTiles, int zTiles, float texTileX, float texTileZ) :
            MeshComponent(go, new Mesh)
        {
            uint32_t vertexCount = xTiles * zTiles * 6;

            float oneOverXTiles = 1.0f / (float)xTiles;
            float oneOverZTiles = 1.0f / (float)zTiles;
            vertices.resize(vertexCount * 3);
            texCoords.resize(vertexCount * 4);

            for (uint32_t x = 0; x < xTiles; x++)
            {
                for (uint32_t z = 0; z < zTiles; z++)
                {
                    uint32_t tileIndex = (x * zTiles + z) * 6;
                    float xBeginTile = (oneOverXTiles * (float)x) * texTileX;
                    float xEndTile = (oneOverXTiles * (float)(x + 1)) * texTileX;
                    float zBeginTile = (oneOverZTiles * (float)z) * texTileZ;
                    float zEndTile = (oneOverZTiles * (float)(z + 1)) * texTileZ;

                    float xBegin = (oneOverXTiles * (float)x);
                    float xEnd = (oneOverXTiles * (float)(x + 1));
                    float zBegin = (oneOverZTiles * (float)z);
                    float zEnd = (oneOverZTiles * (float)(z + 1));

                    vertices[tileIndex + 0] = static_cast<float>(x);
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z);
                    texCoords[tileIndex + 0] = xBeginTile;
                    texCoords[tileIndex + 1] = zBeginTile;
                    texCoords[tileIndex + 2] = xBegin;
                    texCoords[tileIndex + 3] = zBegin;
                    tileIndex++;

                    vertices[tileIndex + 0] = static_cast<float>(x);
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[tileIndex + 0] = xBeginTile;
                    texCoords[tileIndex + 1] = zBeginTile;
                    texCoords[tileIndex + 2] = xBegin;
                    texCoords[tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z);
                    texCoords[tileIndex + 0] = xEndTile;
                    texCoords[tileIndex + 1] = zBeginTile;
                    texCoords[tileIndex + 2] = xEnd;
                    texCoords[tileIndex + 3] = zBegin;
                    tileIndex++;

                    vertices[tileIndex + 0] = static_cast<float>(x);
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[tileIndex + 0] = xBeginTile;
                    texCoords[tileIndex + 1] = zEndTile;
                    texCoords[tileIndex + 2] = xBegin;
                    texCoords[tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z) + 1.f;
                    texCoords[tileIndex + 0] = xEndTile;
                    texCoords[tileIndex + 1] = zEndTile;
                    texCoords[tileIndex + 2] = xEnd;
                    texCoords[tileIndex + 3] = zEnd;
                    tileIndex++;

                    vertices[tileIndex + 0] = static_cast<float>(x) + 1.f;
                    vertices[tileIndex + 1] = 0.f;
                    vertices[tileIndex + 2] = static_cast<float>(z);
                    texCoords[tileIndex + 0] = xEndTile;
                    texCoords[tileIndex + 1] = zBeginTile;
                    texCoords[tileIndex + 2] = xEnd;
                    texCoords[tileIndex + 3] = zBegin;
                    tileIndex++;
                }
            }
        }

        virtual void init() override {
            mMesh->addVertexBuffer(VertexType::Position, 0, 3);
            mMesh->addVertexBuffer(VertexType::Texture0, 1, 4);
        }

    };
}