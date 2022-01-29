#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Texture1D.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class AOShader : public PostProcessShader {

    public:

        float radius = 0.925f;
        float bias = 0.5f;

        AOShader(const std::string &frag) :
            PostProcessShader("AO Shader", frag) {

            // generate kernel
            Texture *kernelTex = Library::createEmptyTexture<Texture1D>("aoKernel", { GL_RGB16F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE });
            generateKernel(32);

            // generate 4x4 noise texture
            Texture *noiseTex = Library::createEmptyTexture<Texture2D>("aoNoise", { GL_RGB16F, GL_RGB, GL_NEAREST, GL_REPEAT, GL_UNSIGNED_BYTE });
            generateNoise(4);
        }

        void generateKernel(unsigned size) {
            std::vector<uint8_t> kernel;
            for (unsigned i = 0; i < size; i++) {
                glm::vec3 sample(
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom(0.f, 1.f)
                );
                sample = glm::normalize(sample);
                sample *= Util::genRandom(0.f, 1.f);
                float scale = (float)i / (float)size;
                scale = Util::lerp(0.1f, 1.f, scale * scale);
                sample *= scale;
                kernel.push_back(static_cast<uint8_t>(sample.x));
                kernel.push_back(static_cast<uint8_t>(sample.y));
                kernel.push_back(static_cast<uint8_t>(sample.z));
            };
            Library::getTexture("aoKernel")->update(glm::uvec2(size, 1), kernel.data());
        }

        void generateNoise(unsigned dim) {
            std::vector<uint8_t> noise;
            noise.resize(dim*dim*3);
            for (unsigned i = 0; i < dim*dim*3; i+=3) {
                noise[i + 0] = Util::genRandom();
                noise[i + 1] = Util::genRandom();
                noise[i + 2] = Util::genRandom();
            }
            Library::getTexture("aoNoise")->update(glm::uvec2(dim), noise.data());
        }

        virtual void render() override {

            loadUniform("radius", radius);
            loadUniform("bias", bias);

            // bind gbuffer
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDepth",  *gbuffer->mTextures[2]);

            // bind kernel and noise
            loadTexture("noise", *Library::getTexture("aoNoise"));
            loadTexture("kernel", *Library::getTexture("aoKernel"));

            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
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