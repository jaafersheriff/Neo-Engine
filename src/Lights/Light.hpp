/* General point light class */
#pragma once
#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include "glm/glm.hpp"

class Light {
    public:
        /* Point light attributes */
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 attenuation;

        /* Constructors */
        Light(const glm::vec3 position, const glm::vec3 color, const glm::vec3 attenuation) {
            this->position = position;
            this->color = color;
            this->attenuation = attenuation;
        }

        Light(const glm::vec3 position, const glm::vec3 color) :
            Light(position, color, glm::vec3(1.f, 0.f, 0.f))
        {
        }

        Light(const glm::vec3 position) :
            Light(position, glm::vec3(1.f))
        {
        }

        Light() :
            Light(glm::vec3(0.f))
        {
        }
};

#endif