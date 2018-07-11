#pragma once

#include "System/System.hpp"

namespace neo {

    class RenderSystem : public System {
        public:
            virtual void init() override;
            virtual void update(float) override;
    };
}