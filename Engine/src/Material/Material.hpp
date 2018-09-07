#pragma once 

#include <glm/glm.hpp>

namespace neo {

    class Material {

        public:

            Material(const float amb, const glm::vec3 & dif, const glm::vec3 & spec, const float shine) :
                ambient(amb),
                diffuse(dif),
                specular(spec),
                shine(shine)
            {}

            float ambient;
            glm::vec3 diffuse;
            glm::vec3 specular;
            float shine;
    };

}