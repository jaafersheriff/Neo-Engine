#include "ECS.hpp"

#include <imgui/imgui.h>

namespace neo {

    void ECS::_updateSystems() {
        MICROPROFILE_SCOPEI("ECS", "_updateSystems", MP_AUTO);
        for (auto& system : mSystems) {
            if (system.second->mActive) {
                MICROPROFILE_SCOPEI(system.second->mName.c_str(), "update", MP_AUTO);
                system.second->update(*this);
            }
        }
    }

    void ECS::clean() {
        for (auto& gameObject : mGameObjects) {
            removeGameObject(*gameObject);
        }
        _processKillQueue();
    }

    GameObject& ECS::createGameObject(const std::string& tag) {
        mGameObjectInitQueue.emplace_back(std::make_unique<GameObject>(tag));
        return *mGameObjectInitQueue.back().get();
    }

    void ECS::removeGameObject(GameObject& go) {
        mGameObjectKillQueue.push_back(&go);
    }

    void ECS::_removeComponent(std::type_index type, Component* component) {
        assert(mComponents.count(type)); // trying to remove a type of component that was never added
        mComponentKillQueue.emplace_back(type, component);
    }

    void ECS::_processInitQueue() {
        MICROPROFILE_SCOPEI("ECS", "_processKillQueue()", MP_AUTO);
        _initGameObjects();
        _initComponents();
    }

    void ECS::_initGameObjects() {
        for (auto& object : mGameObjectInitQueue) {
            mGameObjects.emplace_back(std::move(object));
        }
        mGameObjectInitQueue.clear();
    }

    void ECS::_initComponents() {

        for (int i = 0; i < int(mComponentInitQueue.size()); i++) {
            auto& type(mComponentInitQueue[i].first);
            auto& comp(mComponentInitQueue[i].second);
            /* Add Component to respective GameObjects */
            comp.get()->getGameObject().addComponent(*comp.get(), type);

            /* Add Component to active engine */
            auto it(mComponents.find(type));
            if (it == mComponents.end()) {
                mComponents.emplace(type, std::make_unique<std::vector<std::unique_ptr<Component>>>());
                it = mComponents.find(type);
            }
            it->second->emplace_back(std::move(comp));
            it->second->back()->init();
        }
        mComponentInitQueue.clear();
    }

    void ECS::_initSystems() {
        for (auto& system : mSystems) {
            system.second->init(*this);
        }
    }

    void ECS::_processKillQueue() {
        MICROPROFILE_SCOPEI("ECS", "_processKillQueue()", MP_AUTO);
        /* Remove Components from GameObjects */
        for (auto& comp : mComponentKillQueue) {
            comp.second->getGameObject().removeComponent(*(comp.second), comp.first);
        }

        _killGameObjects();
        _killComponents();
    }

    void ECS::_killGameObjects() {
        for (auto killIt(mGameObjectKillQueue.begin()); killIt != mGameObjectKillQueue.end(); ++killIt) {
            bool found = false;
            /* Look in active game objects in reverse order */
            for (int i = int(mGameObjects.size()) - 1; i >= 0; --i) {
                GameObject* go(mGameObjects[i].get());
                if (go == *killIt) {
                    /* Add game object's components to kill queue */
                    for (auto compTIt(go->mComponentsByType.begin()); compTIt != go->mComponentsByType.end(); ++compTIt) {
                        for (auto& comp : compTIt->second) {
                            comp->removeGameObject();
                            mComponentKillQueue.emplace_back(compTIt->first, comp);
                        }
                    }
                    mGameObjects.erase(mGameObjects.begin() + i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                /* Look in game object initialization queue, in reverse order */
                for (int i = int(mGameObjectInitQueue.size()) - 1; i >= 0; --i) {
                    if (mGameObjectInitQueue[i].get() == *killIt) {
                        mGameObjectInitQueue.erase(mGameObjectInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        mGameObjectKillQueue.clear();
    }

    void ECS::_killComponents() {
        for (auto& killE : mComponentKillQueue) {
            std::type_index typeI(killE.first);
            Component* comp(killE.second);
            comp->kill();
            bool found(false);
            /* Look in active components in reverse order */
            if (mComponents.count(typeI)) {
                auto& comps(*mComponents.at(typeI));
                for (int i = int(comps.size()) - 1; i >= 0; --i) {
                    if (comps[i].get() == comp) {
                        auto it(comps.begin() + i);
                        comps.erase(it);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                /* Look in component initialization queue in reverse order */
                for (int i = int(mComponentInitQueue.size()) - 1; i >= 0; --i) {
                    if (mComponentInitQueue[i].second.get() == comp) {
                        mComponentInitQueue.erase(mComponentInitQueue.begin() + i);
                        break;
                    }
                }
            }
        }
        mComponentKillQueue.clear();
    }

    void ECS::_imguiEdtor() {
        size_t count = 0;
        for (auto go : getGameObjects()) {
            count += go->getAllComponents().size();
        }
        ImGui::Text("Components: %d", count);
        char buf[256];
        sprintf(buf, "Gameobjects: %d", static_cast<int>(getGameObjects().size()));
        if (ImGui::TreeNodeEx(buf)) {
            for (auto& gameObject : getGameObjects()) {
                if (gameObject->mTag.size() && ImGui::TreeNodeEx(gameObject->mTag.c_str())) {
                    for (auto& component : gameObject->getAllComponents()) {
                        component->imGuiEditor();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (mSystems.size() && ImGui::TreeNodeEx("Systems", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (unsigned i = 0; i < mSystems.size(); i++) {
                auto& sys = mSystems[i].second;
                ImGui::PushID(i);
                bool treeActive = ImGui::TreeNodeEx(sys->mName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("SYSTEM_SWAP", &i, sizeof(unsigned));
                    ImGui::Text("Swap %s", sys->mName.c_str());
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("SYSTEM_SWAP")) {
                        IM_ASSERT(payLoad->DataSize == sizeof(unsigned));
                        unsigned payload_n = *(const unsigned*)payLoad->Data;
                        mSystems[i].swap(mSystems[payload_n]);
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
                if (treeActive) {
                    ImGui::Checkbox("Active", &sys->mActive);
                    sys->imguiEditor(*this);
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }
}