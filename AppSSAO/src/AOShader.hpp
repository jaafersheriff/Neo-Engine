#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "GLObjects/Texture1D.hpp"

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
            Texture *kernelTex = Library::createEmptyTexture<Texture1D>("aoKernel", { GL_RGB16F, GL_RGB, GL_NEAREST, GL_REPEAT });
            generateKernel(32);

            // generate 4x4 noise texture
            Texture *noiseTex = Library::createEmptyTexture<Texture2D>("aoNoise", { GL_RGB16F, GL_RGB, GL_NEAREST, GL_REPEAT });
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
            Texture *kernelTex = Library::getTexture("aoKernel");
            kernelTex->resize(glm::uvec2(size, 1));
            kernelTex->upload(kernel.data());
        }

        void generateNoise(unsigned dim) {
            std::vector<uint8_t> noise;
            noise.resize(dim*dim*3);
            for (unsigned i = 0; i < dim*dim*3; i+=3) {
                noise[i + 0] = Util::genRandom();
                noise[i + 1] = Util::genRandom();
                noise[i + 2] = Util::genRandom();
            }
            Texture *noiseTex = Library::getTexture("aoNoise");
            noiseTex->bind();
            noiseTex->resize(glm::uvec2(dim));
            noiseTex->upload(noise.data());
        }

        virtual void render(const CameraComponent &camera) override {

            loadUniform("radius", radius);
            loadUniform("bias", bias);

            // bind gbuffer
            auto gbuffer = Library::getFBO("gbuffer");
            gbuffer->mTextures[0]->bind();
            loadUniform("gNormal", gbuffer->mTextures[0]->mTextureID);
            gbuffer->mTextures[2]->bind();
            loadUniform("gDepth", gbuffer->mTextures[2]->mTextureID);

            // bind kernel and noise
            auto noise = Library::getTexture("aoNoise");
            noise->bind();
            loadUniform("noise", noise->mTextureID);
            auto kernel = Library::getTexture("aoKernel");
            kernel->bind();
            loadUniform("kernel", kernel->mTextureID);

            loadUniform("P", camera.getProj());
            loadUniform("invP", glm::inverse(camera.getProj()));
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