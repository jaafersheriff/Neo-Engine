#include "GameObject.hpp"

#include "Component/Component.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    GameObject::GameObject() :
        mComponents(),
        mComponentsByType(),
        mSpatial(nullptr)
    {}

    void GameObject::addComponent(Component & component, std::type_index typeI) {
        mComponents.push_back(&component);
        mComponentsByType[typeI].push_back(&component);
        if (!mSpatial && typeI == std::type_index(typeid(SpatialComponent))) {
            mSpatial = dynamic_cast<SpatialComponent *>(&component);
        }
    }

    void GameObject::removeComponent(Component & component, std::type_index typeI) {
        /* Remove from allComponents */
        for (auto it(mComponents.begin()); it != mComponents.end(); ++it) {
            if (*it == &component) {
                mComponents.erase(it);
                break;
            }
        }
        /* Remove from compsByCompT in reverse order */
        auto compsIt(mComponentsByType.find(typeI));
        if (compsIt != mComponentsByType.end()) {
            auto & comps(compsIt->second);
            for (int i(int(comps.size()) - 1); i >= 0; --i) {
                if (comps[i] == &component) {
                    comps.erase(comps.begin() + i);
                    break;
                }
            }
        }
        if (mSpatial == &component) {
            mSpatial = nullptr;
        }
    }
}