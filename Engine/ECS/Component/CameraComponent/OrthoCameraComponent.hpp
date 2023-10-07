#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"

namespace neo {

    struct OrthoCameraComponent : public CameraComponent {
            OrthoCameraComponent(float near, float far, float hMin, float hMax, float vMin, float vMax);

            virtual std::string getName() const override { return "OrthoCameraComponent"; }
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