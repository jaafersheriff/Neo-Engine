#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

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
            Texture *kernelTex = Library::getEmptyTexture("aoKernel");
            kernelTex->mWidth = 32;
            kernelTex->mHeight = 1;
            kernelTex->mComponents = 3;
            CHECK_GL(glGenTextures(1, &kernelTex->mTextureID));
            kernelTex->bind();
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, 64, 0, GL_RGB, GL_FLOAT, nullptr));
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            CHECK_GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            CHECK_GL(glBindTexture(GL_TEXTURE_1D, 0));
            assert(glGetError() == GL_NO_ERROR);;
            generateKernel(32);

            // generate 4x4 noise texture
            Texture *noiseTex = Library::getEmptyTexture("aoNoise");
            noiseTex->mFormat = { GL_RGB16F, GL_RGB, GL_NEAREST, GL_REPEAT };
            noiseTex->mWidth = noiseTex->mHeight = 4;
            noiseTex->mComponents = 3;
            noiseTex->upload();
            generateNoise(4);
        }

        void generateKernel(unsigned size) {
            std::vector<glm::vec3> kernel;
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
                kernel.push_back(sample);
            };
            Texture *kernelTex = Library::getTexture("aoKernel");
            kernelTex->mWidth = size;
            kernelTex->mHeight = 1;
            kernelTex->bind();
            CHECK_GL(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, kernel.size(), 0, GL_RGB, GL_FLOAT, &kernel[0]));
        }

        void generateNoise(unsigned dim) {
            std::vector<glm::vec3> noise;
            for (unsigned i = 0; i < dim*dim; i++) {
                noise.push_back(glm::normalize(glm::vec3(
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom(-1.f, 1.f),
                    0.f
                )));
            }
            Texture *noiseTex = Library::getTexture("aoNoise");
            noiseTex->mWidth = noiseTex->mHeight = dim;
            noiseTex->bind();
            CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, dim, dim, 0, GL_RGB, GL_FLOAT, &noise[0]));
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
};