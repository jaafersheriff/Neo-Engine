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
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
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

            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            if (cameraView) {
                auto&& [cameraEntity, __, cameraSpatial] = *cameraView;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", cameraSpatial.getView());
            }


            if (auto&& volumeOpt = ecs.getSingleView<TagComponent, VolumeComponent, OrthoCameraComponent, SpatialComponent>()) {
                auto&& [_, __, volume, ortho, cameraSpat] = *volumeOpt;

                size_t numVoxels = volume.mTexture->mWidth * volume.mTexture->mHeight * volume.mTexture->mDepth;
                auto dimension = 0x1 << volume.mSize;
                glm::vec2 xBounds = ortho.getHorizontalBounds();
                glm::vec2 yBounds = ortho.getVerticalBounds();
                glm::vec2 zBounds = ortho.getNearFar();
                glm::vec3 range = glm::vec3(
                    xBounds.y - xBounds.x,
                    yBounds.y - yBounds.x,
                    zBounds.y - zBounds.x);
                glm::vec3 voxelSize = range / static_cast<float>(dimension);
                loadUniform("voxelSize", voxelSize);

                /* Pull volume data out of GPU */
                float* voxelData = new float[numVoxels * 4];
                volume.mTexture->bind();
                {
                    RENDERER_MP_ENTER("Read volume");
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    // pretty sure the amd issue is here?
                    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, voxelData);
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    RENDERER_MP_LEAVE();
                }

                std::vector<float> voxelPositions;
                voxelPositions.reserve(numVoxels * 3);
                std::vector<float> voxelColors;
                voxelColors.reserve(numVoxels * 4);

                RENDERER_MP_ENTER("Create instance data");
                for (int i = 0; i < numVoxels; i++) {
                    float r = voxelData[i * 4 + 0];
                    float g = voxelData[i * 4 + 1];
                    float b = voxelData[i * 4 + 2];
                    float a = voxelData[i * 4 + 3];
                    if (a <= 0.05f) {
                        continue;
                    }
                    voxelColors.push_back(r);
                    voxelColors.push_back(g);
                    voxelColors.push_back(b);
                    voxelColors.push_back(a);

                    glm::ivec3 index = volume.getVoxelIndex(i);

                    // TODO - this needs to be pushed back by 1/2 voxel size
                    float x = float(index.x) * range.x / dimension + xBounds.x + voxelSize.x / 2.f;
                    float y = float(index.y) * range.y / dimension + yBounds.x + voxelSize.y / 2.f;
                    float z = float(index.z) * range.z / dimension + zBounds.x + voxelSize.z / 2.f;
                    glm::vec3 pos = cameraSpat.getPosition() + glm::vec3(x, y, z);
                    pos -= static_cast<float>(dimension);
                    pos *= -1;
                    voxelPositions.push_back(x);
                    voxelPositions.push_back(y);
                    voxelPositions.push_back(z);
                }
                delete[] voxelData;
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
