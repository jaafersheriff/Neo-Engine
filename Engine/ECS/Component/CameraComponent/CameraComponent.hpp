#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct CameraComponent : public Component {
		CameraComponent();
		virtual std::string getName() const override { return "CameraComponent"; }
		virtual void imGuiEditor() override;

		/* Setters */
		void setNearFar(float, float);

		/* Getters */
		const glm::vec2 getNearFar() const { return glm::vec2(mNear, mFar); }
		const glm::mat4& getProj() const;

	protected:
		float mNear, mFar;

		virtual void _detProj() const = 0;

		/* Should never be used directly -- call getters */
		mutable glm::mat4 mProjMat;
		mutable bool mProjMatDirty;

	};

}