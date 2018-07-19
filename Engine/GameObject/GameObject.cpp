#include "GameObject.hpp"

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    GameObject::GameObject() :
        allComponents(),
        compsByCompT(),
        spatial(nullptr)
    {}

    void GameObject::addComponent(Component & component, std::type_index typeI) {
        allComponents.push_back(&component);
        compsByCompT[typeI].push_back(&component);
        if (!spatial && typeI == std::type_index(typeid(SpatialComponent))) {
            spatial = dynamic_cast<SpatialComponent *>(&component);
        }
    }

    void GameObject::removeComponent(Component & component, std::type_index typeI) {
        /* Remove from allComponents */
        for (auto it(allComponents.begin()); it != allComponents.end(); ++it) {
            if (*it == &component) {
                allComponents.erase(it);
                break;
            }
        }
        /* Remove from compsByCompT in reverse order */
        auto compsIt(compsByCompT.find(typeI));
        if (compsIt != compsByCompT.end()) {
            auto & comps(compsIt->second);
            for (int i(int(comps.size()) - 1); i >= 0; --i) {
                if (comps[i] == &component) {
                    comps.erase(comps.begin() + i);
                    break;
                }
            }
        }
        if (spatial == &component) {
            spatial = nullptr;
        }
    }
}