#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Library.hpp"

#include "Util/Util.hpp"

using namespace neo;

namespace Sponza {

    namespace {
        void _generateKernel(uint32_t size) {
            std::vector<float> kernel;
            for (unsigned i = 0; i < size; i++) {
                glm::vec3 sample(
                    util::genRandom(-1.f, 1.f),
                    util::genRandom(-1.f, 1.f),
                    util::genRandom(0.f, 1.f)
                );
                sample = glm::normalize(sample);
                sample *= util::genRandom(0.f, 1.f);
                float scale = (float)i / (float)size;
                scale = util::lerp(0.1f, 1.f, scale * scale);
                sample *= scale;
                // texture is 32bit, data upload is 8bit
                kernel.push_back(sample.x);
                kernel.push_back(sample.y);
                kernel.push_back(sample.z);
            };
            Library::createTexture("aoKernel", { TextureTarget::Texture1D, GL_RGB32F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE }, glm::uvec3(size, 0, 0), reinterpret_cast<uint32_t*>(kernel.data()));
        }

        void _generateNoise(uint32_t dim) {
            std::vector<float> noise;
            noise.resize(dim * dim * 3);
            for (unsigned i = 0; i < dim * dim * 3; i += 3) {
                noise[i + 0] = util::genRandom();
                noise[i + 1] = util::genRandom();
                noise[i + 2] = util::genRandom();
            }
            Library::createTexture("aoNoise", { TextureTarget::Texture2D, GL_RGB32F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE }, glm::uvec3(dim, dim, 0), reinterpret_cast<uint32_t*>(noise.data()));
        }
    }

     Framebuffer* drawAO(const ECS& ecs, ECS::Entity cameraEntity, Framebuffer& gbuffer, glm::uvec2 targetSize, float radius, float bias) {
        TRACY_GPU();
        if (!Library::hasTexture("aoKernel")) {
            _generateKernel(8);
        }
        if (!Library::hasTexture("aoNoise")) {
            _generateNoise(4);
        }

        // Make a one-off framebuffer for the base AO
        // Do base AO at half res
        auto baseAOTarget = Library::getPooledFramebuffer({ glm::max(glm::uvec2(1,1), targetSize / 2u), {
            TextureFormat{
                TextureTarget::Texture2D,
                GL_R16F,
                GL_RED,
                GL_LINEAR,
                GL_REPEAT,
                GL_FLOAT
            },
        } }, "AO base");
        baseAOTarget->bind();
        baseAOTarget->clear(glm::vec4(0.f), GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, targetSize.x / 2u, targetSize.y / 2u);

        {
            TRACY_GPUN("Base AO");
            auto* aoShader = Library::createSourceShader("AOShader", SourceShader::ConstructionArgs{
                { ShaderStage::VERTEX, "quad.vert"},
                { ShaderStage::FRAGMENT, "sponza/ao.frag" }
                });
            auto& resolvedShader = aoShader->getResolvedInstance({});
            resolvedShader.bind();

            resolvedShader.bindUniform("radius", radius);
            resolvedShader.bindUniform("bias", bias);

            // bind gbuffer
            resolvedShader.bindTexture("gNormal", *gbuffer.mTextures[3]);
            resolvedShader.bindTexture("gDepth", *gbuffer.mTextures[4]);

            // bind kernel and noise
            resolvedShader.bindTexture("noise", *Library::getTexture("aoNoise"));
            resolvedShader.bindTexture("kernel", *Library::getTexture("aoKernel"));

            const auto P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
            resolvedShader.bindUniform("P", P);
            resolvedShader.bindUniform("invP", glm::inverse(P));

            Library::getMesh("quad").mMesh->draw();
        }

        {
            TRACY_GPUN("AO Blur");
            // Do base AO at full res?
            auto blurredAO = Library::getPooledFramebuffer({ targetSize, {
                TextureFormat{
                    TextureTarget::Texture2D,
                    GL_R16F,
                    GL_RED,
                    GL_LINEAR,
                    GL_REPEAT,
                    GL_FLOAT
                },
            } }, "AO blurred");
            blurredAO->bind();
            blurredAO->clear(glm::vec4(0.f), GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, targetSize.x, targetSize.y);

            auto* blurShader = Library::createSourceShader("BlurShader", SourceShader::ConstructionArgs{
                { ShaderStage::VERTEX, "quad.vert"},
                { ShaderStage::FRAGMENT, "sponza/blur.frag" }
                });
            auto& resolvedShader = blurShader->getResolvedInstance({});
            resolvedShader.bind();

            resolvedShader.bindTexture("inputAO", *baseAOTarget->mTextures[0]);
            resolvedShader.bindUniform("blurAmount", 2);

            Library::getMesh("quad").mMesh->draw();
            return blurredAO;
        }
    }

}