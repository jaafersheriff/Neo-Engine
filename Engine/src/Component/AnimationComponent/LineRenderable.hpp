#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"
#include "LineComponent.hpp"

#include "Util/GLHelper.hpp"

namespace neo {

    class LineRenderable : public RenderableComponent {

        public:

            LineRenderable(GameObject *go, LineComponent *line) :
                RenderableComponent(go, new Mesh),
                line(line)
            {}

            virtual void init() override;

            virtual const Mesh & getMesh() const override;

            LineComponent * line;

    };
}