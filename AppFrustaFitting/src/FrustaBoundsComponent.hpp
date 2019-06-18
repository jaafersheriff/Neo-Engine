#pragma once

using namespace neo;

class FrustaBoundsComponent : public Component {
    public:
        struct BoundingBox {
            glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());

            void setMinMax(glm::vec3 other) {
                if (other.x < min.x) { min.x = other.x; }
                if (other.y < min.y) { min.y = other.y; }
                if (other.z < min.z) { min.z = other.z; }
                if (other.x > max.x) { max.x = other.x; }
                if (other.y > max.y) { max.y = other.y; }
                if (other.z > max.z) { max.z = other.z; }
            }
        };

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

        const BoundingBox getBoundingBox() const {

            glm::vec3 offset(0.f);
            if (auto spatial = this->getGameObject().getSpatial()) {
                offset = spatial->getPosition();
            }

            BoundingBox bb;
            bb.setMinMax(FarLeftBottom - offset);
            bb.setMinMax(FarLeftTop - offset);
            bb.setMinMax(FarRightBottom - offset);
            bb.setMinMax(FarRightTop - offset);
            bb.setMinMax(NearLeftBottom - offset);
            bb.setMinMax(NearLeftTop - offset);
            bb.setMinMax(NearRightBottom - offset);
            bb.setMinMax(NearRightTop - offset);

            return bb;
        }

    private:
        
};