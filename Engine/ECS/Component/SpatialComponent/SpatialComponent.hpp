#pragma once

#include "ECS/Component/Component.hpp"
#include "Orientable.hpp"

#include <glm/glm.hpp>

namespace neo {

	START_COMPONENT(SpatialComponent);
		SpatialComponent();
		SpatialComponent(const glm::vec3&);
		SpatialComponent(const glm::vec3&, const glm::vec3&);
		SpatialComponent(const glm::vec3&, const glm::vec3&, const glm::vec3&);
		SpatialComponent(const glm::vec3&, const glm::vec3&, const glm::mat3&);
	
		virtual void imGuiEditor() override;
	
		/* Update */
		void move(const glm::vec3&);
		void resize(const glm::vec3&);
		void rotate(const glm::mat3&);
	
		/* Setters */
		void setPosition(const glm::vec3&);
		void setScale(const glm::vec3&);
		void setScale(const float);
		void setOrientation(const glm::mat3&);
		void setLookDir(const glm::vec3&);
		void setModelMatrix(const glm::mat4&);
		void setUVW(const glm::vec3&, const glm::vec3&, const glm::vec3&);
		void setDirty();
	
		/* Getters */
		const Orientable& getOrientable() const { return mOrientable; }
		glm::vec3 getPosition() const { return mPosition; }
		glm::vec3 getScale() const { return mScale; }
		const glm::mat4& getModelMatrix() const;
		const glm::mat3& getNormalMatrix() const;
		const glm::mat4& getView() const;

	private:
		glm::vec3 mPosition{ 0.f, 0.f, 0.f };
		glm::vec3 mScale{ 1.f, 1.f, 1.f };
		
		void _detModelMatrix() const;
		void _detNormalMatrix() const;
		void _detView() const;
		mutable Orientable mOrientable;
		mutable glm::mat4 mModelMatrix;
		mutable glm::mat3 mNormalMatrix;
		mutable glm::mat4 mViewMat;
		mutable bool mModelMatrixDirty;
		mutable bool mNormalMatrixDirty;
		mutable bool mViewMatDirty;
	END_COMPONENT();

};

