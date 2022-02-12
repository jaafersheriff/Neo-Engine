#pragma once

#include <imgui/imgui.h>

namespace neo {

    class GameObject;

    class Component {

        public:
            Component(GameObject *go) : mGameObject(go) {};

            /* Overridden functions */
            virtual void init() {};
            virtual void kill() {};
            /* Components can have an editor */
            virtual void imGuiEditor() {};

            /* Remove copy constructors */
            Component(const Component &) = delete;
            Component & operator=(const Component &) = delete;
            Component(Component &&) = default;
            Component & operator=(Component &&) = default;
            
            /* Virtual destructor necessary for polymorphic destruction */
            virtual ~Component() = default;


            /* GameObject */
            GameObject & getGameObject() { return *mGameObject; }
            const GameObject& getGameObject() const { return *mGameObject; }
            void removeGameObject() { mGameObject = nullptr; }
                 
        protected:
            GameObject* mGameObject;
    };
}