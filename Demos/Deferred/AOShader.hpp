#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Texture1D.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

namespace Deferred {
    class AOShader : public PostProcessShader {

    public:

        float radius = 0.4f;
        float bias = 0.4f;

        AOShader(const std::string& frag) :
            PostProcessShader("AO Shader", frag) {

            // generate kernel
            Texture* kernelTex = Library::createEmptyTexture<Texture2D>("aoKernel", { GL_RGB32F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE });
            NEO_UNUSED(kernelTex);
            generateKernel(8);

            // generate 4x4 noise texture
            Texture* noiseTex = Library::createEmptyTexture<Texture2D>("aoNoise", { GL_RGB32F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE });
            NEO_UNUSED(noiseTex);
            generateNoise(4);
        }

        void generateKernel(unsigned size) {
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
            Library::getTexture("aoKernel")->update(glm::uvec2(size, 1), reinterpret_cast<uint32_t*>(kernel.data()));
        }

        void generateNoise(unsigned dim) {
            std::vector<float> noise;
            noise.resize(dim * dim * 3);
            for (unsigned i = 0; i < dim * dim * 3; i += 3) {
                noise[i + 0] = util::genRandom();
                noise[i + 1] = util::genRandom();
                noise[i + 2] = util::genRandom();
            }
            Library::getTexture("aoNoise")->update(glm::uvec2(dim), reinterpret_cast<uint32_t*>(noise.data()));
        }

        virtual void render(const ECS& ecs) override {

            loadUniform("radius", radius);
            loadUniform("bias", bias);

            // bind gbuffer
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDepth", *gbuffer->mTextures[2]);

            // bind kernel and noise
            loadTexture("noise", *Library::getTexture("aoNoise"));
            loadTexture("kernel", *Library::getTexture("aoKernel"));

            if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("invP", glm::inverse(camera->get<CameraComponent>()->getProj()));
            }
        }

        virtual void imguiEditor() override {
            int size = Library::getTexture("aoKernel")->mWidth;
            if (ImGui::SliderInt("Kernel", &size, 1, 128)) {
                generateKernel(size);
            }
            size = Library::getTexture("aoNoise")->mWidth;
            if (ImGui::SliderInt("Noise", &size, 1, 32)) {
                generateNoise(size);
            }
            ImGui::SliderFloat("Radius", &radius, 0.f, 1.f);
            ImGui::SliderFloat("Bias", &bias, 0.f, 1.f);
        }
    };
}