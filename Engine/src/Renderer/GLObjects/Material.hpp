#pragma once

#include <glm/glm.hpp>

namespace neo {

    struct Material {
        glm::vec3 ambient = glm::vec3(1.f);
        glm::vec3 diffuse = glm::vec3(0.f);
        glm::vec3 specular = glm::vec3(1.f);
        float shininess = 1.f;
        // float transmittance[3]; // Unused
        // float emission[3]; // Unused
        // float ior;      // index of refraction - Unused
        // float dissolve; // 1 == opaque; 0 == fully transparent
    };
}