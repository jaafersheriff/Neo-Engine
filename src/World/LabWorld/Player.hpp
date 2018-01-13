#pragma once
#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "Camera/Camera.hpp"
#include "BoundingBox/BoundingBox.hpp"

class Player {
    public:
        Player(Camera *, BoundingBox);

        Camera *camera;
        BoundingBox boundingBox;

        void update();
};

#endif
