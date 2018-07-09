#pragma once

class GameObject {

    public:
        /* Don't copy GameObjects */
        GameObject(const GameObject &) = delete;
        GameObject(GameObject &&) = delete;
        GameObject & operator=(const GameObject &) = delete;
};