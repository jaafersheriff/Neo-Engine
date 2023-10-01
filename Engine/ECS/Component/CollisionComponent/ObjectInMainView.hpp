#pragma once

#include "ECS/Component/Component.hpp"
namespace neo {

    struct ObjectInMainViewComponent : public Component {
        virtual std::string getName() const override { return "ObjectInMainViewComponent"; } 
    };
}