#include "EditorSystem.hpp"
#include <Engine.hpp>

namespace neo {

    EditorSystem::EditorSystem() : SelectingSystem() {
        Engine::addImGuiFunc("Editor", []() {
            if (ImGui::Button("Create GameObject")) {
                Engine::removeComponent<SelectedComponent>(*Engine::getSingleComponent<SelectedComponent>());
                auto& go = Engine::createGameObject();
                Engine::addComponent<BoundingBoxComponent>(&go, std::vector<float>{ -1.f, -1.f, -1.f, 1.f, 1.f, 1.f });
                Engine::addComponent<SpatialComponent>(&go);
                Engine::addComponent<SelectableComponent>(&go);
                Engine::addComponent<SelectedComponent>(&go);
                Engine::addComponent<MeshComponent>(&go, Library::getMesh("sphere"));
                Engine::addComponent<renderable::WireframeRenderable>(&go);
            }
            if (auto selected = Engine::getSingleComponent<SelectedComponent>()) {
                auto allComponents = selected->getGameObject()._getcomps();
                static std::optional<std::type_index> index;
                if (ImGui::BeginCombo("", "Edit components")) {
                    index = std::nullopt;
                    for (auto comp : allComponents) {
                        if (ImGui::Selectable(comp.first.name() + 6)) {
                            index = comp.first;
                        }

                    }
                    ImGui::EndCombo();
                }
                if (index) {
                    auto components = allComponents[index.value()];
                    if (components.size()) {
                        static int offset = 0;
                        if (components.size() > 1) {
                            ImGui::SliderInt("Index", &offset, 0, components.size() - 1);
                        }
                        components[offset]->imGuiEditor();
                    }
                }
                if (ImGui::BeginCombo("", "Add components")) {
                    // TODO..
                    ImGui::EndCombo();
                }
            }
        });
    }
}
