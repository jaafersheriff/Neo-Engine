#pragma once

#include "ECS/Component/Component.hpp"

#include <optional>

namespace neo {

	START_COMPONENT(CameraComponent);
		struct Perspective {
			float mFOV;
			float mAspectRatio;

			bool operator==(const Perspective& other) const {
				return mFOV == other.mFOV && mAspectRatio == other.mAspectRatio;
			}

			bool imGuiEditor();
		};

		struct Orthographic {
			glm::vec2 mHorizBounds;
			glm::vec2 mVertBounds;
			bool operator==(const Orthographic& other) const {
				return mHorizBounds == other.mHorizBounds && mVertBounds == other.mVertBounds;
			}

			bool imGuiEditor();
		};

		enum class CameraType {
			Perspective,
			Orthographic
		};

		CameraComponent(float near, float far, Perspective inCamera);
		CameraComponent(float near, float far, Orthographic inCamera);

		void setNear(float near);
		void setFar(float far);
		void setPerspective(Perspective camera);
		void setOrthographic(Orthographic camera);

		CameraType getType() const;
		float getNear() const;
		float getFar() const;
		const Perspective& getPerspective() const;
		const Orthographic& getOrthographic() const;
		const glm::mat4& getProj() const;
		virtual void imGuiEditor() override;

	private:
		CameraComponent(float near, float far, CameraType type);
		std::optional<Perspective> mPerspective;
		std::optional<Orthographic> mOrthographic;
		CameraType mType;

		float mNear, mFar;
	
		/* Should never be used directly -- call getters */
		mutable glm::mat4 mProjMat;
		mutable bool mProjMatDirty = false;

	END_COMPONENT();

}