#pragma once

#include "Renderer/Renderer.hpp"
#include "Util/ServiceLocator.hpp"
#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "VolumeWriteCameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"

#include "Loader/Library.hpp"

#include "ECS/ECS.hpp"

using namespace neo;

namespace Froxels {
    class VolumeWriteShader : public Shader {

    public:

        VolumeWriteShader(const std::string& compute) :
            Shader("VolumeWrite Shader")
        {
            _attachStage(ShaderStage::COMPUTE, compute);
            init();
        }

        virtual void render(const ECS& ecs) override {
            bind();

            if (auto volumeWriteCamera = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent>()) {
                auto&& [_, __, camera] = *volumeWriteCamera;
                loadUniform("near", camera.getNearFar().x);
                loadUniform("far", camera.getNearFar().y);

                auto lastFrameBackbuffer = Library::getFBO("backbuffer");
                loadTexture("inputColor", *lastFrameBackbuffer->mTextures[0]);
                loadTexture("inputDepth", *lastFrameBackbuffer->mTextures[1]);

                auto volume = Library::getTexture("Volume");
                glBindImageTexture(0, volume->mTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

                glDispatchCompute(1920, 1080, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }

            unbind();
        }

        virtual void imguiEditor() override {
        }
    };
}
