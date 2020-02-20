#pragma once

#include <glm/glm.hpp>

namespace neo {

    class Material {
    public:
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        float mShininess;
        glm::vec3 mTransmittance;
        glm::vec3 mEmission;
        float mIOR;      // index of refraction
        float mDissolve; // 1 == opaque; 0 == fully transparent

        Material(glm::vec3 ambient = glm::vec3(1.f), glm::vec3 diffuse = glm::vec3(0.f),
            glm::vec3 specular = glm::vec3(1.f), float shininess = 1.f,
            glm::vec3 transmittance = glm::vec3(0.f), glm::vec3 emission = glm::vec3(0.f),
            float ior = 1.f, float dissolve = 1.f
            ) :
            mAmbient(ambient),
            mDiffuse(diffuse),
            mSpecular(specular),
            mShininess(shininess),
            mTransmittance(transmittance),
            mEmission(emission),
            mIOR(ior),
            mDissolve(dissolve)
        {}
    };
}