#pragma once

using namespace neo;

class FrustaBoundsComponent : public Component {
    public:
        FrustaBoundsComponent(GameObject *go) :
            Component(go) 
        {}

        glm::vec3 NearLeftBottom;
        glm::vec3 NearLeftTop;
        glm::vec3 NearRightBottom;
        glm::vec3 NearRightTop;
        glm::vec3 FarLeftBottom;
        glm::vec3 FarLeftTop;
        glm::vec3 FarRightBottom;
        glm::vec3 FarRightTop;

};