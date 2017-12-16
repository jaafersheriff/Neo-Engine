/* Sun class
 * Simple billboard projecting a sun texture
 * Moves with the main light source in the world */
#pragma once
#ifndef _SUN_HPP_
#define _SUN_HPP_

#include "Billboard/Billboard.hpp"

#include "Light/Light.hpp"
#include "Camera/Camera.hpp"

class Sun : public Billboard {
    public: 
        Sun(Texture *, float);                      /* Textured sun constructor */
        Sun(glm::vec3, glm::vec3, float, float);    /* Untextuerd sun contsructor */
        
    
        /* Move position according to light source */
        void update(const Light &);

        /* Members used if no texture is provided */
        glm::vec3 innerColor;   /* Inner opaque color */
        glm::vec3 outerColor;   /* Border transparent color */
        float innerRadius;      /* Radius of inner opaque region */
        float outerRadius;      /* Radius of outer transprarent region */

};

#endif
