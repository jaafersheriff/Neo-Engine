#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

#include <glm/glm.hpp>

using namespace neo;

class WaterComponent : public Component{

    public:
        struct WaveData {
            glm::vec3 direction = glm::vec3(0.3f, 0.f, -0.7f);
            float steepness = 1.8f;
            float waveLength = 4.f;
            float amplitude = 0.6f;
            float speed = 1.1f;
        };
        std::vector<WaveData> mWaveData;
        bool mDirtyWave = true;

        float mDampening = 0.5f; // per wave?

        glm::vec2 mReflectanceFactor = glm::vec2(0.16f, 0.2f);
        float mRoughness = 0.5f;
        float mSpecIntensity = 0.2f;

        glm::vec3 mBaseColor = glm::vec3(0.14f, 0.25f, 0.87f);

        WaterComponent(GameObject *gameObject) :
            Component(gameObject)
        {}

        virtual void imGuiEditor() override {
            ImGui::SliderFloat3("Color", &mBaseColor[0], 0.f, 1.f);
            ImGui::SliderFloat("Dampening", &mDampening, 0.1f, 5.f);
            ImGui::SliderFloat("Reflectance", &mReflectanceFactor[0], 0.f, 1.f);
            ImGui::SliderFloat("Roughness", &mRoughness, 0.f, 1.f);
            ImGui::SliderFloat("Specular", &mSpecIntensity, 0.f, 1.f);
            if (ImGui::Button("Add wave")) {
                mWaveData.push_back(WaveData{});
                mDirtyWave = true;
            }
            if (ImGui::TreeNodeEx("Wave data", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (int i = 0; i < mWaveData.size(); i++) {
                    ImGui::PushID(i);
                    ImGui::Separator();
                    mDirtyWave |= ImGui::SliderFloat3("Direction", &mWaveData[i].direction[0], -1.f, 1.f);
                    mWaveData[i].direction = glm::normalize(mWaveData[i].direction);
                    mDirtyWave |= ImGui::SliderFloat("Steepness", &mWaveData[i].steepness, 0.01f, 10.f);
                    mDirtyWave |= ImGui::SliderFloat("Wavelength", &mWaveData[i].waveLength, 0.01f, 10.f);
                    mDirtyWave |= ImGui::SliderFloat("Speed", &mWaveData[i].speed, 0.f, 1.f);
                    mDirtyWave |= ImGui::SliderFloat("Amplitude", &mWaveData[i].amplitude, 0.f, 10.f);
                    if (ImGui::Button("Delete")) {
                        mWaveData.erase(mWaveData.begin() + i);
                        mDirtyWave = true;
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
};

