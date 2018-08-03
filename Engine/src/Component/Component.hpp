#pragma once

namespace neo {

    class GameObject;

    class Component {

        public:
            Component(GameObject &go) : gameObject(&go) {};

            /* Overridden functions */
            virtual void init() {};
            virtual void update(float) {};
            virtual void kill() {};

            /* Remove copy constructors */
            Component(const Component &) = delete;
            Component & operator=(const Component &) = delete;
            Component(Component &&) = default;
            Component & operator=(Component &&) = default;
            
            /* Virtual destructor necessary for polymorphic destruction */
            virtual ~Component() = default;

            GameObject & getGameObject() { return *gameObject; }
            const GameObject & getGameObject() const { return *gameObject; }
            void removeGameObject() { gameObject = nullptr; }
                 
        protected:
            GameObject * gameObject;
    };
}