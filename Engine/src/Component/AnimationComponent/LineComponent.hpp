#pragma once

#include "Component/Component.hpp"

#include "Util/GLHelper.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace neo {

    class LineComponent : public Component {

        public:

            LineComponent(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
                Component(go),
                lineColor(color)
            {}

            mutable bool isDirty = false;
            glm::vec3 lineColor;

            const std::vector<glm::vec3> & getNodes() const { return nodes; }

            void addNode(const glm::vec3 &node) {
                nodes.push_back(node);
                isDirty = true;
            }

            void addNodes(const std::vector<glm::vec3> &oNodes) {
                nodes.insert(nodes.end(), oNodes.begin(), oNodes.end());
                isDirty = true;
            }

            void removeNode(const glm::vec3 &node) {
                auto it = std::find(nodes.begin(), nodes.end(), node);
                if (it != nodes.end()) {
                    nodes.erase(it);
                    isDirty = true;
                }
            }

            void removeNode(const int index) {
                if (index >= 0 && index < (int)nodes.size()) {
                    nodes.erase(nodes.begin() + index);
                    isDirty = true;
                }
            }

        private:

            std::vector<glm::vec3> nodes;

    };
}