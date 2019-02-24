#pragma once 

#include <glm/glm.hpp>

namespace neo {

    class Material {

        public:

            Material(const float amb, const glm::vec3 & dif, const glm::vec3 & spec, const float shine) :
                mAmbient(amb),
                mDiffuse(dif),
                mSpecular(spec),
                mShine(shine)
            {}

            float mAmbient;
            glm::vec3 mDiffuse;
            glm::vec3 mSpecular;
            float mShine;
    };

}