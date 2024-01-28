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

    void drawAO(const ECS& ecs, ECS::Entity cameraEntity, Framebuffer& gbuffer, float radius, float bias) {
        if (!Library::hasTexture("aoKernel")) {
            _generateKernel(8);
        }
        if (!Library::hasTexture("aoNoise")) {
            _generateNoise(4);
        }

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

        resolvedShader.bindUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
        resolvedShader.bindUniform("invP", glm::inverse(ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj()));

        Library::getMesh("quad").mMesh->draw();
    }

}