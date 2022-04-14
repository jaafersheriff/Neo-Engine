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

            myMesh.mMesh = new Mesh;
            myMesh.mMesh->mPrimitiveType = GL_POINTS;
            myMesh.mMesh->addVertexBuffer(VertexType::Position, 0, 3, {});
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
                // loadUniform("voxelSize", voxelSize);
                loadUniform("lod", lod);
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
                myMesh.mMesh->updateVertexBuffer(VertexType::Position, voxelPositions);

                myMesh.mMesh->draw();
            }

            unbind();
        }

        int lod = 0;
        int volsize = 1;
        MeshData myMesh;
        virtual void imguiEditor() override {
            ImGui::SliderInt("LOD", &lod, 0, volsize);
        }

    };
}