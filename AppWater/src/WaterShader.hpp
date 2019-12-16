#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

#include "WaterMeshComponent.hpp"

using namespace neo;

class WaterShader : public Shader {

public:

    struct WaveData {
        glm::vec3 direction = glm::vec3(0.3f, 0.f, -0.7f);
        float steepness = 1.8f;
        float waveLength = 4.f;
        float amplitude = 0.6f;
        float speed = 1.1f;
    };
    Texture* waveTextureDirection;
    Texture* waveTextureData;
    std::vector<WaveData> waveData;

    glm::vec3 tessFactor = glm::vec3(10.f, 7.f, 3.f);
    glm::vec2 tessDistance = glm::vec2(2.f, 5.f);
    float dampeningFactor = 0.5f;

    glm::vec4 normalMapScrollDir = glm::vec4(1.f, 0.f, 1.f, 0.f);
    glm::vec2 normalMapScrollSpeed = glm::vec2(1.f, 1.f);

    // glm::vec2 reflectanceFactor = glm::vec2(16.f, 0.2f);
    float shine = 20.f;

    WaterShader(const std::string &vert, const std::string &frag, const std::string& control, const std::string& eval) :
        Shader("Water Shader") {
        _attachType(vert, ShaderType::VERTEX);
        _attachType(frag, ShaderType::FRAGMENT);
        _attachType(control, ShaderType::TESSELLATION_CONTROL);
        _attachType(eval, ShaderType::TESSELLATION_EVAL);
        init();

        TextureFormat directionFormat;
        directionFormat.inputFormat = GL_RGB16F;
        directionFormat.format = GL_RGB;
        directionFormat.filter = GL_LINEAR;
        directionFormat.mode = GL_CLAMP;
        waveTextureDirection = Library::createEmptyTexture<Texture2D>("waveDirection", directionFormat);

        TextureFormat dataFormat;
        dataFormat.inputFormat = GL_RGBA16F;
        dataFormat.format = GL_RGBA;
        dataFormat.filter = GL_LINEAR;
        dataFormat.mode = GL_CLAMP;
        waveTextureData = Library::createEmptyTexture<Texture2D>("wavedata", dataFormat);
    }

    virtual void render(const CameraComponent &camera) override {
        bind();
        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());
        loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

        loadUniform("tessFactor", tessFactor);
        loadUniform("tessDistance", tessDistance);
        loadUniform("time", Util::getRunTime());
        loadUniform("dampeningFactor", dampeningFactor);

        loadTexture("waveDir", *waveTextureDirection);
        loadTexture("waveData", *waveTextureData);
        loadUniform("numWaves", waveData.size());

        loadTexture("WaterNormalMap1", *Library::getTexture("water1.png", { GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT }));
        loadTexture("WaterNormalMap2", *Library::getTexture("water2.png", { GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT }));
        loadUniform("normalMapScrollDir", normalMapScrollDir);
        loadUniform("normalMapScrollSpeed", normalMapScrollSpeed);

        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
        }
        // loadUniform("reflectanceFactor", reflectanceFactor);
        loadUniform("shine", shine);

        CHECK_GL(glPatchParameteri(GL_PATCH_VERTICES, 3));
                // CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        if (auto renderable = Engine::getComponentTuple<WaterMeshComponent, SpatialComponent>()) {
            loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", renderable->get<SpatialComponent>()->getNormalMatrix());

            renderable->get<WaterMeshComponent>()->getMesh().draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat3("TessFactor", &tessFactor[0], 0.1f, 10.f);
        ImGui::SliderFloat2("TessDistance", &tessDistance[0], 0.1f, 30.f);
        ImGui::SliderFloat("Dampening", &dampeningFactor, 0.1f, 5.f);

        ImGui::SliderFloat2("NormalMap1 scroll dir", &normalMapScrollDir[0], 0.f, 1.f);
        ImGui::SliderFloat("NormalMap1 scroll speed", &normalMapScrollSpeed[0], 0.f, 1.f);
        ImGui::SliderFloat2("NormalMap2 scroll dir", &normalMapScrollDir[2], 0.f, 1.f);
        ImGui::SliderFloat("NormalMap2 scroll speed", &normalMapScrollSpeed[1], 0.f, 1.f);

        // ImGui::SliderFloat("Reflectance.x", &reflectanceFactor[0], 1.f, 50.f);
        // ImGui::SliderFloat("Reflectance.y", &reflectanceFactor[1], 0.f, 1.f);
        ImGui::SliderFloat("shine", &shine, 1.f, 50.f);

        bool updateTexture = false;
        if (ImGui::Button("Add wave")) {
            waveData.push_back(WaveData{});
            updateTexture = true;
        }
        if (ImGui::TreeNodeEx("Wave data", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < waveData.size(); i++) {
                ImGui::PushID(i);
                ImGui::Separator();
                updateTexture |= ImGui::SliderFloat3("Direction", &waveData[i].direction[0], -1.f, 1.f);
                waveData[i].direction = glm::normalize(waveData[i].direction);
                updateTexture |= ImGui::SliderFloat("Steepness", &waveData[i].steepness, 0.01f, 10.f);
                updateTexture |= ImGui::SliderFloat("Wavelength", &waveData[i].waveLength, 0.01f, 10.f);
                updateTexture |= ImGui::SliderFloat("Speed", &waveData[i].speed, 0.f, 1.f);
                updateTexture |= ImGui::SliderFloat("Amplitude", &waveData[i].amplitude, 0.f, 10.f);
                if (ImGui::Button("Delete")) {
                    waveData.erase(waveData.begin() + i);
                    updateTexture = true;
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        if (updateTexture && waveData.size()) {
            std::vector<float> rawDirectionData;
            rawDirectionData.resize(waveData.size() * 3);
            std::vector<float> rawWaveData;
            rawWaveData.resize(waveData.size() * 4);
            for (int i = 0; i < waveData.size(); i++) {
                rawDirectionData[3 * i + 0] = waveData[i].direction.x;
                rawDirectionData[3 * i + 1] = waveData[i].direction.y;
                rawDirectionData[3 * i + 2] = waveData[i].direction.z;
                waveTextureDirection->update(glm::uvec2(waveData.size(), 1), rawDirectionData.data());

                rawWaveData[4 * i + 0] = waveData[i].amplitude;
                rawWaveData[4 * i + 1] = waveData[i].speed;
                rawWaveData[4 * i + 2] = waveData[i].steepness;
                rawWaveData[4 * i + 3] = waveData[i].waveLength;
                waveTextureData->update(glm::uvec2(waveData.size(), 1), rawWaveData.data());
            }
        }
    }
};