#pragma once

#include "Loader/Library.hpp"
#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ECS/ECS.hpp"
#include "ECS//Messaging/Messenger.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/MeshGenerator.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace Froxels {
    class VolumeDebugShader : public Shader {

    public:

        VolumeDebugShader(const std::string& vert, const std::string& frag) :
            Shader("VolumeDebugShader", vert, frag) {

            MeshData mesh;
            prefabs::generateCube(mesh);
            mesh.mMesh->addVertexBuffer(VertexType::Texture1, 3, 3, {}); // Position
            glVertexAttribDivisor(3, 1); 
            mesh.mMesh->addVertexBuffer(VertexType::Texture2, 4, 4, {}); // voxel data -- color
            glVertexAttribDivisor(4, 1); 
            Library::insertMesh("instanced sphere", mesh);
        }

        virtual void render(const ECS& ecs) override {
            bind();

            loadUniform("voxelSize", 1.f);
            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            if (cameraView) {
                auto&& [cameraEntity, __, cameraSpatial] = *cameraView;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", cameraSpatial.getView());
            }


            if (auto&& volumeOpt = ecs.getSingleView<TagComponent, VolumeComponent>()) {
                auto&& [_, __, volume] = *volumeOpt;

                size_t numVoxels = volume.mTexture->mWidth * volume.mTexture->mHeight * volume.mTexture->mDepth;

                /* Pull volume data out of GPU */
                std::vector<uint32_t> voxelData;
                voxelData.resize(numVoxels);
                volume.mTexture->bind();
                glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, voxelData.data());

                std::vector<glm::vec3> voxelPositions;
                voxelPositions.reserve(numVoxels);
                std::vector<glm::vec4> voxelColors;
                voxelColors.reserve(numVoxels);

                int _c = 0; // im so lazy
                for (size_t x = 0; x < volume.mTexture->mWidth; x++) {
                    for (size_t y = 0; y < volume.mTexture->mHeight; y++) {
                        for (size_t z = 0; z < volume.mTexture->mDepth; z++) {
                            uint32_t data = voxelData[_c++];
                            if (data == 0) {
                                continue;
                            }

                            glm::vec4 color;
                            color.x = static_cast<float>(data & 0xFF000000);
                            color.y = static_cast<float>(data & 0x00FF0000);
                            color.z = static_cast<float>(data & 0x0000FF00);
                            color.a = static_cast<float>(data & 0x000000FF);
                            voxelColors.push_back(color);

                            glm::vec3 position = { x, y, z };
                            voxelPositions.push_back(position);
                        }
                    }
                }

                auto instancedMesh = Library::getMesh("instanced sphere");
                instancedMesh.mMesh->updateVertexBuffer(VertexType::Texture1, voxelPositions.data(), voxelPositions.size());
                instancedMesh.mMesh->updateVertexBuffer(VertexType::Texture2, voxelColors.data(), voxelColors.size());

                instancedMesh.mMesh->draw(_c);
            }
            unbind();
        }

    private:
    };
}
