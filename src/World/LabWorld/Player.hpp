#pragma once
#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "Camera/Camera.hpp"
#include "Collision/BoundingSphere.hpp";

class Player {
    public:
        Player(Camera *, BoundingSphere);

        Camera *camera;
        BoundingSphere boundingSphere;

        void update();
};

#endif
