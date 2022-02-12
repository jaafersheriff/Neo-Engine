#pragma once

#include "ECS/GameObject.hpp"
#include "ECS/Component/Component.hpp"

#include <optional>
#include <unordered_map>

namespace neo {

    class Engine;

    class ComponentTuple {

        friend Engine;
        friend ECS;

    public:
        const GameObject& mGameObject;

        ComponentTuple(const GameObject& go) :
            mGameObject(go),
            mValid(true)
        {}

        operator bool() const {
            return mValid;
        }

        template <typename CompT>
        CompT* get() {
            const auto& comp = mComponentMap.find(typeid(CompT));
            NEO_ASSERT(comp != mComponentMap.end(), "Attempting to access an invalid component type");
            return static_cast<CompT *>(comp->second);
        }

        template <typename CompT>
        CompT const* get() const {
            const auto& comp = mComponentMap.find(typeid(CompT));
            NEO_ASSERT(comp != mComponentMap.end(), "Attempting to access an invalid component type");
            return static_cast<CompT const*>(comp->second);
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
