#pragma once

#include "Component/Component.hpp"
#include "Component/CameraComponent/CameraComponent.hpp"

#include <glm/glm.hpp>

namespace neo {

    class OrthoCameraComponent : public CameraComponent {

        public:
            OrthoCameraComponent(GameObject *, float near, float far, float hMin, float hMax, float vMin, float vMax);
            OrthoCameraComponent(OrthoCameraComponent &&) = default;

            virtual void imGuiEditor() override;

            /* Setters */
            void setOrthoBounds(const glm::vec2 &, const glm::vec2 &);

            /* Getters */
            const glm::vec2 getHorizontalBounds() const { return mHorizBounds; }
            const glm::vec2 getVerticalBounds() const { return mVertBounds; }

        private:
            glm::vec2 mHorizBounds;
            glm::vec2 mVertBounds;

            virtual void _detProj() const override;

    };

}