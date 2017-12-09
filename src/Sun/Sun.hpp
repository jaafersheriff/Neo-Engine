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
        /* Reference to light source */
        Light *light;

        /* Constructors */
        Sun(Texture *);
        Sun(Texture *, float);
    
        /* Move position according to light source */
        void update(const Camera &);
};

#endif
