#pragma once

#include "Loader/Library.hpp"
#include "Renderer/Renderer.hpp"
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
                std::vector<glm::vec4> voxelData;
                voxelData.resize(numVoxels);
                volume.mTexture->bind();
                {
                    RENDERER_MP_ENTER("Read volume");
                    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, voxelData.data());
                    RENDERER_MP_LEAVE();
                }

                std::vector<glm::vec3> voxelPositions;
                voxelPositions.reserve(numVoxels);
                std::vector<glm::vec4> voxelColors;
                voxelColors.reserve(numVoxels);

                size_t _c = numVoxels - 1; // im so lazy
                RENDERER_MP_ENTER("Create instance data");
                for (size_t x = 0; x < volume.mTexture->mWidth; x++) {
                    for (size_t y = 0; y < volume.mTexture->mHeight; y++) {
                        for (size_t z = 0; z < volume.mTexture->mDepth; z++) {
                            uint32_t data = voxelData[_c--];
                            glm::vec4 color;
                            color.a = static_cast<float>((data & 0xFF000000) >> 24) / 255.f;
                            color.z = static_cast<float>((data & 0x00FF0000) >> 16) / 255.f;
                            color.y = static_cast<float>((data & 0x0000FF00) >>  8) / 255.f;
                            color.x = static_cast<float>((data & 0x000000FF) >>  0) / 255.f;
                            if (color.a <= 0.2f) {
                                continue;
                            }
                            voxelColors.push_back(color);

                            glm::vec3 position = { x, y, z };
                            voxelPositions.push_back(position);
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
