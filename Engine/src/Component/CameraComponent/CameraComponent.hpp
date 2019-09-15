#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class CameraComponent : public Component {

        public:
            CameraComponent(GameObject *gameObject);
            CameraComponent(CameraComponent &&) = default;

            virtual void init() override;

            /* Setters */
            void setNearFar(float, float);

            /* Getters */
            const glm::vec2 getNearFar() const { return glm::vec2(mNear, mFar); }
            const glm::mat4 & getView() const;
            const glm::mat4 & getProj() const;

        protected:
            float mNear, mFar;

            void _detView() const;
            virtual void _detProj() const = 0;

            /* Should never be used directly -- call getters */
            mutable glm::mat4 mViewMat;
            mutable glm::mat4 mProjMat;
            mutable bool mViewMatDirty;
            mutable bool mProjMatDirty;

    };

}