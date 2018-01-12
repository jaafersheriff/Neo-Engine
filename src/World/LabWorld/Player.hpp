#pragma once
#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "Camera/Camera.hpp"
#include "AABB/AABB.hpp"

class Player {
    public:
        Player(Camera *, AABB);

        Camera *camera;
        AABB boundingBox;

        void update();
};

#endif
