#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"
#include "LineComponent.hpp"

#include "GLHelper/GLHelper.hpp"

namespace neo {

    class LineRenderable : public RenderableComponent {

        public:

            LineRenderable(GameObject *go, LineComponent *line) :
                RenderableComponent(go, new Mesh),
                mLine(line)
            {}

            virtual void init() override;

            virtual const Mesh & getMesh() const override;

            LineComponent * mLine;

    };
}