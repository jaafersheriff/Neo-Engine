#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class SpatialComponent;

    class CameraComponent : public Component {

        public:
            CameraComponent(GameObject *gameObject, float near, float far);
            CameraComponent(GameObject *, float fov, float near, float far);
            CameraComponent(GameObject *, float hMin, float hMax, float vMin, float vMax, float near, float far);
            CameraComponent(CameraComponent &&) = default;

            virtual void init() override;

            /* Setters */
            void setFOV(float);
            void setNearFar(float, float);
            void setOrthoBounds(const glm::vec2 &, const glm::vec2 &);
            void setLookDir(glm::vec3);

            /* Getters */
            const float getFOV() const { return mFOV; }
            const float getNear() const { return mNear; }
            const float getFar() const { return mFar; }
            const glm::vec2 getHorizontalBounds() const { return mHorizBounds; }
            const glm::vec2 getVerticalBounds() const { return mVertBounds; }
            const glm::vec3 getLookDir() const;
            const glm::mat4 & getView() const;
            const glm::mat4 & getProj() const;

        private:
            float mFOV;
            float mNear, mFar;
            glm::vec2 mHorizBounds;
            glm::vec2 mVertBounds;
            bool mIsOrtho;

            void _detView() const;
            void _detProj() const;

            /* Should never be used directly -- call getters */
            mutable glm::mat4 mViewMat;
            mutable glm::mat4 mProjMat;
            mutable bool mViewMatDirty;
            mutable bool mProjMatDirty;

    };

}