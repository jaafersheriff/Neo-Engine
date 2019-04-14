#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class MaterialComponent : public Component {

        public:
            float mAmbient;
            glm::vec3 mDiffuse;
            glm::vec3 mSpecular;
            float mShine;

            MaterialComponent(GameObject *go, const float amb = 0.2f, const glm::vec3 diffuse = glm::vec3(1.f), const glm::vec3 spec = glm::vec3(0.f), const float shine = 1.f) :
                Component(go),
                mAmbient(amb),
                mDiffuse(diffuse),
                mSpecular(spec),
                mShine(shine)
            {}

    };
}