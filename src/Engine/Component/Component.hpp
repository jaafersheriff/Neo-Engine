#pragma once

#include "GameObject/GameObject.hpp"

namespace neo {

    class Component {

        public:
            Component(GameObject &go) : gameObject(&go) {};
            virtual void init() = 0;

            /* Remove copy constructors */
            Component(const Component &) = delete;
            Component(Component &&) = delete;
            Component & operator=(const Component &) = delete;
            Component & operator=(Component &&) = delete;
            
            /* Virtual destructor necessary for polymorphic destruction */
            virtual ~Component() = default;
            virtual void update(float) = 0;

            GameObject & getGameObject() { return *gameObject; }
            const GameObject & getGameObject() const { return *gameObject; }
            void removeGameObject() { gameObject = nullptr; }
                 
        private:
            GameObject * gameObject;
    };
}