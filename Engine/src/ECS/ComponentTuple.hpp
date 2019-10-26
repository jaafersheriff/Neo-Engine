#pragma once

#include "GameObject/GameObject.hpp"
#include "Component/Component.hpp"

#include <optional>
#include <unordered_map>

namespace neo {

    class Engine;

    class ComponentTuple {

        friend Engine;

    public:
        GameObject& gameObject;
        std::unordered_map<std::type_index, Component *> components;
        bool valid;

        ComponentTuple(GameObject& go) :
            gameObject(go),
            valid(true)
        {}

        operator bool() const {
            return valid;
        }

        template <typename CompT>
        CompT* get() {
            return dynamic_cast<CompT*>(components[typeid(CompT)]);
        }

        template <typename CompT, typename... CompTs> 
        void populate() {
            _addComponent<CompT>();
            if constexpr (sizeof...(CompTs) > 0) {
                populate<CompTs...>();
            }
        }

    private:
        template <typename CompT = void> 
        void _addComponent() {
            if (auto comp = gameObject.getComponentByType<CompT>()) {
                components[typeid(CompT)] = comp;
            }
            else {
                valid = false;
            }

        }
    };
}
