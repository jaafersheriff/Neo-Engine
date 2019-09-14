#pragma once

#include <unordered_map>
#include <functional>
#include <typeindex>

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

            /* Add/remove components */
            template<typename CompT> void addComponent(CompT &);
            void addComponent(Component &, std::type_index);
            void removeComponent(Component &, std::type_index);

            /* Get all components by type */
            template <typename CompT> const std::vector<CompT *> & getComponentsByType() const;
            /* Get First component by type */
            template <typename CompT> CompT * getComponentByType() const;

            const std::vector<Component *> getAllComponents() const { return mComponents; }
            int getNumReceiverTypes() { return mReceivers.size(); }
            int getNumReceivers() {
                int count = 0;
                auto it = mReceivers.begin();
                while (it != mReceivers.end()) {
                    count += it->second.size();
                    it++;
                }
                return count;
            }

        private:
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
        static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
        static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

        auto it(mComponentsByType.find(std::type_index(typeid(CompT))));
        if (it != mComponentsByType.end() && it->second.size()) {
            return static_cast<CompT *>(it->second.front());
        }
        return nullptr;
    }
}