#pragma once

#include "Engine.hpp"
#include "ParticleMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class ParticlesComputeShader : public Shader {

public:

    int groupSizeWidth = 64;
    Texture3D* noiseTex;

    ParticlesComputeShader(const std::string &compute) :
        Shader("ParticlesCompute Shader", compute)
    {
        noiseTex = new Texture3D;
        noiseTex->mFormat.filter = GL_LINEAR;
        noiseTex->mFormat.mode = GL_REPEAT;
        noiseTex->mFormat.format = GL_RGBA;
        noiseTex->mFormat.inputFormat = GL_RGBA8_SNORM;
        noiseTex->mComponents = 4;
        noiseTex->resize(glm::uvec3(16));

        uint8_t* data = new uint8_t[16 * 16 * 16 * 4];
        uint8_t* ptr = data;
        for (int i = 0; i < 16 * 16 * 16 * 4; i++) {
            data[i] = rand() & 0xff;
        }

        noiseTex->upload(true, &data);
    }

    virtual void render(const CameraComponent &) override {
        bind();

        if (auto mesh = Engine::getSingleComponent<ParticleMeshComponent>()) {
            auto position = mesh->mMesh->getVBO(VertexType::Position);
            auto velocity = mesh->mMesh->getVBO(VertexType::Color0);

            noiseTex->bind();
            loadUniform("noiseTex3D", noiseTex->mTextureID);
            loadUniform("invNoiseSize", 1 / 16.f);

            // Bind mesh
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, velocity.attribArray, velocity.vboID));

            // Dispatch 
            // TODO - get max groupsize from GL
            CHECK_GL(glDispatchCompute(mesh->mNumVerts / groupSizeWidth, 1, 1));
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));

            // Reset bind
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, velocity.attribArray, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderInt("GroupSize", &groupSizeWidth, 1, 128);
    }
};
