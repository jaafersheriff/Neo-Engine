#pragma once

#include "Loader/Library.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/Shader.hpp"
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
            Library::insertMesh("instanced cube", mesh);
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
                std::vector<float> voxelData;
                voxelData.resize(numVoxels * 4);
                volume.mTexture->bind();
                {
                    RENDERER_MP_ENTER("Read volume");
                    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, voxelData.data());
                    RENDERER_MP_LEAVE();
                }

                std::vector<float> voxelPositions;
                voxelPositions.reserve(numVoxels * 3);
                std::vector<float> voxelColors;
                voxelColors.reserve(numVoxels * 4);

                size_t _c = 0; // im so lazy
                RENDERER_MP_ENTER("Create instance data");
                for (size_t x = 0; x < volume.mTexture->mWidth; x++) {
                    for (size_t y = 0; y < volume.mTexture->mHeight; y++) {
                        for (size_t z = 0; z < volume.mTexture->mDepth; z++) {
                            float r = voxelData[_c * 4 + 0];
                            float g = voxelData[_c * 4 + 1];
                            float b = voxelData[_c * 4 + 2];
                            float a = voxelData[_c * 4 + 3];
                            _c++;
                            if (a <= 0.05f) {
                                // continue;
                            }
                            voxelColors.push_back(r);
                            voxelColors.push_back(g);
                            voxelColors.push_back(b);
                            voxelColors.push_back(a);

                            voxelPositions.push_back(static_cast<float>(x));
                            voxelPositions.push_back(static_cast<float>(y));
                            voxelPositions.push_back(static_cast<float>(z));
                        }
                    }
                }
                RENDERER_MP_LEAVE();

                RENDERER_MP_ENTER("Upload instance data");
                auto instancedMesh = Library::getMesh("instanced cube");
                instancedMesh.mMesh->updateVertexBuffer(VertexType::Texture1, voxelPositions.data(), voxelPositions.size());
                instancedMesh.mMesh->updateVertexBuffer(VertexType::Texture2, voxelColors.data(), voxelColors.size());
                RENDERER_MP_LEAVE();

                RENDERER_MP_ENTER("Draw instance");
                instancedMesh.mMesh->draw(static_cast<uint32_t>(voxelPositions.size()));
                RENDERER_MP_LEAVE();
            }
            unbind();
        }

    private:
    };
}
