#pragma once

#include "Component/Component.hpp"

#include "GLObjects/GLHelper.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace neo {

    class LineComponent : public Component {

        public:

            glm::vec3 mColor;
            std::vector<glm::vec3> mNodes;
            mutable bool mDirty;

            LineComponent(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
                Component(go),
                mColor(color),
                mDirty(false)
            {}

            const std::vector<glm::vec3> & getNodes() const { return mNodes; }

            void addNode(const glm::vec3 &node) {
                mNodes.push_back(node);
                mDirty = true;
            }

            void addNodes(const std::vector<glm::vec3> &oNodes) {
                mNodes.insert(mNodes.end(), oNodes.begin(), oNodes.end());
                mDirty = true;
            }

            void removeNode(const glm::vec3 &node) {
                auto it = std::find(mNodes.begin(), mNodes.end(), node);
                if (it != mNodes.end()) {
                    mNodes.erase(it);
                    mDirty = true;
                }
            }

            void removeNode(const int index) {
                if (index >= 0 && index < (int)mNodes.size()) {
                    mNodes.erase(mNodes.begin() + index);
                    mDirty = true;
                }
            }

    };
}