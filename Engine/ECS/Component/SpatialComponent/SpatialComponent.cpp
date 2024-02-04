#include "ECS/pch.hpp"
#include "SpatialComponent.hpp"

// #include "ECS/Messaging/Messenger.hpp"

namespace neo {

	SpatialComponent::SpatialComponent()
		: Orientable()
		, mPosition(0.f)
		, mScale(1.f)
		, mModelMatrix(1.f)
		, mNormalMatrix(1.f)
		, mViewMat(1.f)
		, mModelMatrixDirty(true)
		, mNormalMatrixDirty(true)
		, mViewMatDirty(true)
	{}

	SpatialComponent::SpatialComponent(const glm::vec3 & p) :
		SpatialComponent() {
		mPosition = p;
	}

	SpatialComponent::SpatialComponent(const glm::vec3 & p, const glm::vec3 & s) :
		SpatialComponent(p) {
		mScale = s;
	}

	SpatialComponent::SpatialComponent(const glm::vec3 & p, const glm::vec3 & s, const glm::vec3 & r) :
		SpatialComponent(p) {
		mScale = s;
		glm::mat4 rotation(1.f);
		rotation = glm::rotate(rotation, r.x, glm::vec3(1, 0, 0));
		rotation = glm::rotate(rotation, r.y, glm::vec3(0, 1, 0));
		rotation = glm::rotate(rotation, r.z, glm::vec3(0, 0, 1));
		setOrientation(glm::mat3(rotation));
	}

	SpatialComponent::SpatialComponent(const glm::vec3 & p, const glm::vec3 & s, const glm::mat3 & o) :
		SpatialComponent(p, s) {
		setOrientation(o);
	}

	void SpatialComponent::move(const glm::vec3 & delta) {
		if (glm::length(delta) == 0.f) {
			return;
		}

		mPosition += delta;
		mModelMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::resize(const glm::vec3& factor) {
		if (factor == glm::vec3(1.f)) {
			return;
		}

		mScale *= glm::clamp(factor, glm::vec3(0.f), factor);
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::rotate(const glm::mat3 & mat) {
		Orientable::rotate(mat);
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::setPosition(const glm::vec3 & loc) {
		if (mPosition == loc) {
			return;
		}

		mPosition = loc;
		mModelMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::setScale(const glm::vec3 & scale) {
		if (this->mScale == scale) {
			return;
		}

		this->mScale = scale;
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::setScale(const float scale) {
		setScale(glm::vec3(scale));
	}

	void SpatialComponent::setOrientation(const glm::mat3 & orient) {
		Orientable::setOrientation(orient);
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::setUVW(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) {
		Orientable::setUVW(u, v, w);
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
		mViewMatDirty = true;
		// Messenger::sendMessage<SpatialChangeMessage>(&mEntity, *this);
	}

	void SpatialComponent::setDirty() {
		mModelMatrixDirty = true;
		mNormalMatrixDirty = true;
	}
		
	const glm::mat4 & SpatialComponent::getModelMatrix() const {
		if (mModelMatrixDirty) {
			_detModelMatrix();
		}
		return mModelMatrix;
	}

	const glm::mat3 & SpatialComponent::getNormalMatrix() const {
		if (mNormalMatrixDirty) {
			_detNormalMatrix();
		}
		return mNormalMatrix;
	}

	const glm::mat4 & SpatialComponent::getView() const {
		if (mViewMatDirty) {
			_detView();
		}
		return mViewMat;
	}

	void SpatialComponent::_detModelMatrix() const {
		TRACY_ZONE();
		mModelMatrix = glm::scale(glm::translate(glm::mat4(1.f), mPosition) * glm::mat4(getOrientation()), mScale);
		mModelMatrixDirty = false;
	}

	void SpatialComponent::_detNormalMatrix() const {
		TRACY_ZONE();
		if (mScale.x == mScale.y && mScale.y == mScale.z) {
			mNormalMatrix = glm::mat3(getModelMatrix());
		}
		else {
			mNormalMatrix = getOrientation() * glm::mat3(glm::scale(glm::mat4(1.f), 1.0f / mScale));
		}
		mNormalMatrixDirty = false;
	}

	void SpatialComponent::_detView() const {
		TRACY_ZONE();
		mViewMat = glm::lookAt(getPosition(), getPosition() + getLookDir(), getUpDir());
		mViewMatDirty = false;
	}

	void SpatialComponent::imGuiEditor() {
		glm::vec3 moveAmount(0.f);
		glm::vec3 scaleAmount(1.f);
		glm::vec3 lookDir = getLookDir();
		ImGui::Text("Position: %0.2f, %0.2f, %0.2f", mPosition[0], mPosition[1], mPosition[2]);
		if (ImGui::DragFloat3("Move", &moveAmount[0], 0.5f, -20.f, 20.f)) {
			move(moveAmount);
		}
		ImGui::Text("Scale: %0.2f, %0.2f, %0.2f", mScale[0], mScale[1], mScale[2]);
		if (ImGui::DragFloat3("Scale", &scaleAmount[0], 0.05f, 0.001f, 2.f)) {
			resize(scaleAmount);
		}
		if (ImGui::DragFloat3("LookDir", &lookDir[0], 0.1f, -1.f, 1.f)) {
			setLookDir(lookDir);
		}
	}
}
