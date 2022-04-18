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
    class VolumeDebugTraceShader : public Shader {

    public:
        VolumeDebugTraceShader(const std::string& vert, const std::string& frag) :
            Shader("VolumeDebugTrace Shader", vert, frag)
        {

            TextureFormat format = { GL_RGB8, GL_RGB, GL_LINEAR, GL_CLAMP_TO_EDGE };
            {
                auto fbo = Library::createFBO("debugTrace");
                fbo->attachColorTexture({ 1, 1 }, format);
                fbo->initDrawBuffers();
            }

            Messenger::addReceiver<FrameSizeMessage>([&](const Message& msg) {
                glm::ivec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
                if (auto front = Library::getFBO("debugTrace")) {
                    front->resize(frameSize);
                }
            });

        }

        virtual void render(const ECS& ecs) override {
            bind();

            if (auto volumeCamera = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, cameraSpatial] = *volumeCamera;
                loadUniform("P", camera.getProj());
                loadUniform("near", camera.getNearFar().x);
                loadUniform("far", camera.getNearFar().y);
                loadUniform("fov", glm::radians(camera.getFOV()));
                loadUniform("ar", camera.getAspectRatio());

                loadUniform("V", cameraSpatial.getView());
                loadUniform("lookDir", glm::normalize(cameraSpatial.getLookDir()));
                loadUniform("upDir", glm::normalize(cameraSpatial.getUpDir()));
                loadUniform("rightDir", glm::normalize(cameraSpatial.getRightDir()));
            }

            auto quad = Library::getMesh("quad");
            auto debugTrace = Library::getFBO("debugTrace");
            if (debugTrace) {
                debugTrace->bind();
                glViewport(0, 0, debugTrace->mTextures[0]->mWidth, debugTrace->mTextures[0]->mHeight);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                loadUniform("bufferSize", glm::vec2(debugTrace->mTextures[0]->mWidth, debugTrace->mTextures[0]->mHeight));
                quad.mMesh->draw();
            }

            unbind();
        }

    };
}