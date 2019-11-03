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
        GameObject& mGameObject;

        ComponentTuple(GameObject& go) :
            mGameObject(go),
            mValid(true)
        {}

        /* Remove copy constructors */
        ComponentTuple(const ComponentTuple &) = delete;
        ComponentTuple & operator=(const ComponentTuple &) = delete;
        ComponentTuple(ComponentTuple &&) = default;
        ComponentTuple & operator=(ComponentTuple &&) = default;

        operator bool() const {
            return mValid;
        }

        template <typename CompT>
        CompT* get() {
            return static_cast<CompT*>(mComponentMap[typeid(CompT)]);
        }

        template <typename CompT, typename... CompTs> 
        void populate() {
            _addComponent<CompT>();
            if constexpr (sizeof...(CompTs) > 0) {
                if (mValid) {
                    populate<CompTs...>();
                }
            }
        }

    private:
        std::unordered_map<std::type_index, Component *> mComponentMap;
        bool mValid;

        template <typename CompT = void> 
        void _addComponent() {
            if (auto comp = mGameObject.getComponentByType<CompT>()) {
                mComponentMap[typeid(CompT)] = comp;
            }
            else {
                mValid = false;
            }

        }
    };
}
