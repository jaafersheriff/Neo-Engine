#pragma once

#include <unordered_map>
#include <functional>
#include <typeindex>

#include "microprofile.h"

namespace neo {

    class Engine;
    class Messenger;
    class Component;
    struct Message;

    class GameObject {

        friend Engine;
        friend Messenger;

        public:
            /* Don't copy GameObjects */
            GameObject(const GameObject &) = delete;
            GameObject(GameObject &&) = delete;
            GameObject & operator=(const GameObject &) = delete;

            GameObject();

            /* Get all components by type */
            template <typename CompT> const std::vector<CompT *> & getComponentsByType() const;
            /* Get First component by type */
            template <typename CompT> CompT * getComponentByType() const;

            const std::vector<Component *> getAllComponents() const { return mComponents; }
            size_t getNumReceiverTypes() { return mReceivers.size(); }
            size_t getNumReceivers() {
                size_t count = 0;
                auto it = mReceivers.begin();
                while (it != mReceivers.end()) {
                    count += it->second.size();
                    it++;
                }
                return count;
            }

        private:
            /* Used by the engine */
            template<typename CompT> void addComponent(CompT &);
            void addComponent(Component &, std::type_index);
            void removeComponent(Component &, std::type_index);
            std::unordered_map<std::type_index, std::vector<Component *>> getComponentsMap() { return mComponentsByType ; }

            /* Containers */
            std::vector<Component *> mComponents;
            std::unordered_map<std::type_index, std::vector<Component *>> mComponentsByType;
            std::unordered_map<std::type_index, std::vector<std::function<void (const Message &)>>> mReceivers;
    };

    /* Template implementation */
    template <typename CompT>
    void GameObject::addComponent(CompT & component) {
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        addComponent(component, std::type_index(typeid(CompT)));
    }

    template <typename CompT>
    const std::vector<CompT *> & GameObject::getComponentsByType() const {
        MICROPROFILE_SCOPEI("GameObject", "getComponentsByType", MP_AUTO);
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        static const std::vector<CompT *> emptyList;

        auto it(mComponentsByType.find(std::type_index(typeid(CompT))));
        if (it != mComponentsByType.end()) {
            return reinterpret_cast<const std::vector<CompT *> &>(it->second);
        }
        return emptyList;
    }

    template <typename CompT>
    CompT * GameObject::getComponentByType() const {
        MICROPROFILE_SCOPEI("GameObject", "getComponentByType", MP_AUTO);
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        auto it(mComponentsByType.find(std::type_index(typeid(CompT))));
        if (it != mComponentsByType.end() && it->second.size()) {
            return static_cast<CompT *>(it->second.front());
        }
        return nullptr;
    }
}