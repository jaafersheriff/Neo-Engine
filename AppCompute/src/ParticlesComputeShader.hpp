#pragma once

#include "Engine.hpp"
#include "ParticleMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"
#include "GLObjects/Texture3D.hpp"

using namespace neo;

class ParticlesComputeShader : public Shader {

public:

    int groupSizeWidth = 64;
    Texture* noiseTex;

    float noiseFreq = 10.f;
    float noiseStrength = 0.01f;
    float damping = 0.95f;

    bool attractorEnabled = false;
    float attractorSpeed = 0.2f;
    float attractorStrength = 0.0002f;
    glm::vec3 baseAttractor = glm::vec3(0.f);

    ParticlesComputeShader(const std::string &compute) :
        Shader("ParticlesCompute Shader", compute)
    {

        TextureFormat format;
        format.filter = GL_LINEAR;
        format.mode = GL_REPEAT;
        format.format = GL_RGBA;
        format.inputFormat = GL_RGBA8_SNORM;
        noiseTex = Library::createEmptyTexture<Texture3D>("noiseTex", format);
        noiseTex->resize(glm::uvec3(16));
        _generateNoiseTexture();
    }

    void _generateNoiseTexture() {
        uint8_t* data = new uint8_t[16 * 16 * 16 * 4];
        uint8_t* ptr = data;
        for (int i = 0; i < 16 * 16 * 16 * 4; i++) {
            data[i] = rand() & 0xff;
        }

        noiseTex->upload(data);

        delete[] data;
    }

    virtual void render(const CameraComponent &) override {
        bind();

        if (auto mesh = Engine::getSingleComponent<ParticleMeshComponent>()) {
            auto position = mesh->mMesh->getVBO(VertexType::Position);
            auto velocity = mesh->mMesh->getVBO(VertexType::Color0);

            noiseTex->bind();
            loadUniform("noiseTex3D", noiseTex->mTextureID);
            loadUniform("invNoiseSize", 1 / 16.f);
            loadUniform("noiseFreq", noiseFreq);
            loadUniform("noiseStrength", noiseStrength);
            loadUniform("damping", damping);

            glm::vec4 attractor(0.f);
            if (attractorEnabled) {
                attractor.x = baseAttractor.x + glm::sin(Util::getRunTime()/1000.f * attractorSpeed);
                attractor.y = baseAttractor.y + glm::sin(Util::getRunTime()/1000.f * attractorSpeed * 1.3f);
                attractor.z = baseAttractor.z + glm::cos(Util::getRunTime()/1000.f * attractorSpeed);
                attractor.w = attractorStrength;
            }
            else {
                attractor.w = 0.f;
            }
            loadUniform("attractor", attractor);

            // Bind mesh
            CHECK_GL(glBindVertexArray(mesh->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, position.vboID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, velocity.attribArray, velocity.vboID));

            // Dispatch 
            // TODO - get max groupsize from GL
            CHECK_GL(glDispatchCompute(mesh->mNumVerts / groupSizeWidth, 1, 1));
            CHECK_GL(glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT));
            CHECK_GL(glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT ));


            // Reset bind
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position.attribArray, 0));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, velocity.attribArray, 0));
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderInt("GroupSize", &groupSizeWidth, 1, 128);
        ImGui::SliderFloat("Noise Frequency", &noiseFreq, 0.f, 30.f);
        ImGui::DragFloat("Noise Strength", &noiseStrength, 0.001f, 0.0f, 0.2f);
        ImGui::DragFloat("Damping", &damping, 0.001f, 0.7f, 0.99f);
        ImGui::Checkbox("Attractor", &attractorEnabled);
        if (attractorEnabled) {
            ImGui::SliderFloat3("Base position", &baseAttractor[0], -10.f, 10.f);
            ImGui::SliderFloat("Attractor speed", &attractorSpeed, 0.f, 100.f);
            ImGui::SliderFloat("Attractor strength", &attractorStrength, 0.f, 1.f);
        }
        if (ImGui::Button("Regenerate noise")) {
            _generateNoiseTexture();
        }
    }
};
