#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

#include <glm/glm.hpp>

using namespace neo;

class SnowComponent : public Component{

    public:
        
        SnowComponent(GameObject *gameObject) :
            Component(gameObject),
            mSnowAngle(0.f, 1.f, 0.f),
            mSnowSize(0.36f),
            mSnowColor(0.36f, 0.48f, 0.56f),
            mHeight(0.07f),
            mRimColor(1.f),
            mRimPower(0.25f)
        {}

        virtual void imGuiEditor() override {
            ImGui::SliderFloat("Snow size", &mSnowSize, 1.f, 0.f);
            ImGui::SliderFloat3("Snow color", glm::value_ptr(mSnowColor), 0.f, 1.f);
            ImGui::SliderFloat("Height", &mHeight, 0.f, .25f);
            ImGui::SliderFloat3("Rim color", glm::value_ptr(mRimColor), 0.f, 1.f);
            ImGui::SliderFloat("Rim power", &mRimPower, 0.f, 25.f);
        }

        glm::vec3 mSnowAngle;
        float mSnowSize;
        glm::vec3 mSnowColor;
        float mHeight;
        glm::vec3 mRimColor;
        float mRimPower;
};

