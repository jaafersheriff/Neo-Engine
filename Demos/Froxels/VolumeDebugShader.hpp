#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "VolumeComponent.hpp"

using namespace neo;

namespace Froxels {
    class VolumeDebugGShader : public Shader {

    public:
        VolumeDebugGShader(const std::string& vert, const std::string& geom, const std::string& frag) :
            Shader("VolumeDebugG Shader")
        {
            _attachStage(ShaderStage::VERTEX, vert);
            _attachStage(ShaderStage::GEOMETRY, geom);
            _attachStage(ShaderStage::FRAGMENT, frag);
            init();

            MeshData indicesMesh;
            indicesMesh.mMesh = new Mesh;
            indicesMesh.mMesh->mPrimitiveType = GL_POINTS;
            indicesMesh.mMesh->addVertexBuffer(VertexType::Position, 0, 3, {});
            Library::insertMesh("Volume indices", indicesMesh);
        }

        virtual void render(const ECS& ecs) override {
            bind();

            /* Load PV */
            if (auto main = ecs.getSingleView<MainCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, cameraSpatial] = *main;
                loadUniform("mainP", camera.getProj());
                loadUniform("mainV", cameraSpatial.getView());
            }

            if (auto volumeCamera = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, cameraSpatial] = *volumeCamera;
                loadUniform("persP", camera.getProj());
                loadUniform("camNear", camera.getNearFar().x);
                loadUniform("camFar", camera.getNearFar().y);
                loadUniform("fov", glm::radians(camera.getFOV()));
                loadUniform("ar", camera.getAspectRatio());

                loadUniform("persV", cameraSpatial.getView());
                loadUniform("camPos", cameraSpatial.getPosition());
                loadUniform("lookDir", glm::normalize(cameraSpatial.getLookDir()));
                loadUniform("upDir", glm::normalize(cameraSpatial.getUpDir()));
                loadUniform("rightDir", glm::normalize(cameraSpatial.getRightDir()));
            }

            if (auto&& volumeOpt = ecs.getSingleView<TagComponent, VolumeComponent>()) {
                auto&& [_, __, volume] = *volumeOpt;

                volsize = volume.mSize;
                auto dimension = 0x1 << (volsize - lod);
                size_t numVoxels = dimension * dimension * dimension;
                loadUniform("lod", lod);
                loadUniform("dims", glm::vec3(static_cast<float>(dimension)));
                loadTexture("volume", *volume.mTexture);

                std::vector<float> voxelPositions;
                voxelPositions.resize(numVoxels * 3);
                int _c = 0;
                for (int x = 0; x < dimension; x++) {
                    for (int y = 0; y < dimension; y++) {
                        for (int z = 0; z < dimension; z++) {
                            voxelPositions[_c * 3 + 0] = static_cast<float>(x);
                            voxelPositions[_c * 3 + 1] = static_cast<float>(y);
                            voxelPositions[_c * 3 + 2] = static_cast<float>(z);
                            _c++;
                        }
                    }
                }
                auto mesh = Library::getMesh("Volume indices");
                mesh.mMesh->updateVertexBuffer(VertexType::Position, voxelPositions);

                mesh.mMesh->draw();
            }

            unbind();
        }

        int lod = 0;
        int volsize = 1;
        virtual void imguiEditor() override {
            ImGui::SliderInt("LOD", &lod, 0, volsize);
        }

    };
}