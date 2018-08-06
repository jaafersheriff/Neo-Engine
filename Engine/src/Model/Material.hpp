#pragma once 

#include <glm/glm.hpp>

namespace neo {

    class Material {

        public:

            // Material() :
            //     ambient(0.f),
            //     diffuse(glm::vec3(1.f)),
            //     specular(glm::vec3(1.f)),
            //     shine(20.f)
            // {}

            Material(const float amb = 0.f, const glm::vec3 & dif = glm::vec3(1.f), const glm::vec3 & spec = glm::vec3(1.f), const float shine = 20.f) :
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