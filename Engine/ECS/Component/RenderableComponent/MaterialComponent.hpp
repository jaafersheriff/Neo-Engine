#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
    class Texture; 
	struct MaterialComponent : public Component {
        glm::vec3 mAmbient;

        Texture* mDiffuseMap;
        glm::vec3 mDiffuse;

        glm::vec3 mSpecular;
        float mShininess;
        glm::vec3 mTransmittance;
        glm::vec3 mEmission;
        float mIOR;      // index of refraction
        float mDissolve; // 1 == opaque; 0 == fully transparent

        virtual std::string getName() const override {
            return "MaterialComponent";
        }
	};
}