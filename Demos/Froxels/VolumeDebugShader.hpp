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

                /* Load PV */
                auto cameraView = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>();
                if (cameraView) {
                    auto&& [cameraEntity, _, camera, cameraSpatial] = *cameraView;
                    loadUniform("P", camera.getProj());
                    loadUniform("V", cameraSpatial.getView());
                }

            if (auto&& volumeOpt = ecs.getSingleView<TagComponent, VolumeComponent, OrthoCameraComponent, SpatialComponent>()) {
                auto&& [_, __, volume, ortho, cameraSpat] = *volumeOpt;

                volsize = volume.mSize;
                auto dimension = 0x1 << (volsize - lod);
                size_t numVoxels = dimension * dimension * dimension;
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
                    glGetTexImage(GL_TEXTURE_3D, lod, GL_RGBA, GL_FLOAT, voxelData);
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                    RENDERER_MP_LEAVE();
                }

                std::vector<float> voxelPositions;
                voxelPositions.reserve(numVoxels * 3);
                std::vector<float> voxelColors;
                voxelColors.reserve(numVoxels * 4);

                RENDERER_MP_ENTER("Create instance data");

                if (doTrace) {
                    glm::vec3 base = glm::ivec3(dimension / 2, dimension / 2, 0);
                    for (int x = -resolution / 2; x <= resolution / 2; x++) {
                        for (int y = -resolution / 2; y <= resolution / 2; y++) {
                            glm::vec3 dir;
                            dir.x = x / (static_cast<float>(resolution + 1) / 2);
                            dir.y = y / (static_cast<float>(resolution + 1) / 2);
                            glm::vec3 start = base;
                            start.x += dir.x;
                            start.y += dir.y;
                            dir.z = dimension / maxSteps * stepSize;
                            int step = 0;
                            glm::vec3 end = start;
                            while (step < maxSteps && end.x < dimension && end.y < dimension && end.z < dimension) {
                                step++;
                                size_t index = static_cast<int>(volume.reverseVoxelIndex(end, lod));
                                float r = voxelData[index * 4 + 0];
                                float g = voxelData[index * 4 + 1];
                                float b = voxelData[index * 4 + 2];
                                float a = voxelData[index * 4 + 3];

                                end += dir;
                                if (a <= 0.05f) {
                                    continue;
                                }
                                voxelColors.push_back(r);
                                voxelColors.push_back(g);
                                voxelColors.push_back(b);
                                voxelColors.push_back(a);

                                float _x = float(end.x) * range.x / dimension + xBounds.x + voxelSize.x / 2.f;
                                float _y = float(end.y) * range.y / dimension + yBounds.x + voxelSize.y / 2.f;
                                float _z = float(end.z) * range.z / dimension + zBounds.x + voxelSize.z / 2.f;
                                voxelPositions.push_back(_x);
                                voxelPositions.push_back(_y);
                                voxelPositions.push_back(_z);
                            }
                        }
                    }
                }
                else {
                    for (int i = 0; i < numVoxels; i++) {
                        float r = voxelData[i * 4 + 0];
                        float g = voxelData[i * 4 + 1];
                        float b = voxelData[i * 4 + 2];
                        float a = voxelData[i * 4 + 3];
                        if (a <= 0.05f) {
                            a = 0.25f;
                        }
                        voxelColors.push_back(r);
                        voxelColors.push_back(g);
                        voxelColors.push_back(b);
                        voxelColors.push_back(a);

                        glm::vec3 index = volume.getVoxelIndex(i, lod);
                        // Map [0-dim] to [-dim/2, dim/2]
                        index -= static_cast<float>(dimension) / 2.f;

                        float x = float(index.x) * range.x / dimension + xBounds.x;
                        float y = float(index.y) * range.y / dimension + yBounds.x;
                        float z = float(index.z) * range.z / dimension + zBounds.x;
                        glm::vec3 posIndex = { x, y, z };
                        glm::vec3 pos = cameraSpat.getPosition() + cameraSpat.getLookDir() * range.z / 2.f;
                        pos += glm::vec3(posIndex.x, posIndex.y, posIndex.z);

                        // TODO - this needs to be pushed back by 1/2 voxel size
                        voxelPositions.push_back(pos.x);
                        voxelPositions.push_back(pos.y);
                        voxelPositions.push_back(pos.z);
                    }
                }
                RENDERER_MP_LEAVE();
                delete[] voxelData;

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

        bool doTrace = false;
        int resolution = 1;
        float stepSize = 0.1f;
        int maxSteps = 16;

        int lod = 0;
        int volsize = 1;

        virtual void imguiEditor() override { 
            ImGui::Checkbox("trace", &doTrace);
            if (doTrace) {
                ImGui::SliderInt("resolution", &resolution, 1, 256);
                ImGui::SliderFloat("stepSize", &stepSize, 0.0001f, 10.f);
                ImGui::SliderInt("max steps", &maxSteps, 1, 64);
            }
            else {
            }
            ImGui::SliderInt("LOD", &lod, 0, volsize);
        }
    };
}
