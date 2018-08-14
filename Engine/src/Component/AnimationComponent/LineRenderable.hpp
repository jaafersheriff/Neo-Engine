#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"

#include "Util/GLHelper.hpp"

namespace neo {

    class LineRenderable : public RenderableComponent {

        public:

            LineRenderable(GameObject *go, glm::vec3 color = glm::vec3(1.f)) :
                RenderableComponent(go, new Mesh),
                lineColor(color)
            {}

            virtual void init() override;

            virtual const Mesh & getMesh() const override;

            const std::vector<glm::vec3> & getNodes() const { return nodes; }

            void addNode(const glm::vec3 &);
            void addNodes(const std::vector<glm::vec3> &);
            void removeNode(const glm::vec3 &);
            void removeNode(const int);

            glm::vec3 lineColor;

        private:

            mutable bool isDirty = false;
            std::vector<glm::vec3> nodes;

    };
}