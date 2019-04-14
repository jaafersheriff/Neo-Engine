#pragma once

#include <glm/glm.hpp>

namespace neo {

    class Orientable {

        public:
            glm::vec3 mU, mV, mW;

            Orientable() :
                mU(1.f, 0.f, 0.f),
                mV(0.f, 1.f, 0.f),
                mW(0.f, 0.f, 1.f),
                mOrientation(),
                mOrientationDirty(false)
            {}

            Orientable(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) :
                mU(u),
                mV(v),
                mW(w),
                mOrientation(),
                mOrientationDirty(true)
            {}

            virtual ~Orientable() = default;

            virtual void rotate(const glm::mat3 & mat) {
                mOrientation = mat * mOrientation;
                mOrientationDirty = false;
                _detUVW();
            }

            /* Setters */
            virtual void setOrientation(const glm::mat3 & o) {
                mOrientation = o;
                mOrientationDirty = false;
                _detUVW();
            }
            virtual void setUVW(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) {
                this->mU = glm::normalize(u);
                this->mV = glm::normalize(v);
                this->mW = glm::normalize(w);
                mOrientationDirty = true;
            }

            /* Getters */
            virtual const glm::mat3 & getOrientation() const {
                if (mOrientationDirty) {
                    _detOrientation();
                }
                return mOrientation;
            }

        private:    
            void _detOrientation() const {
                mOrientation = glm::mat3(mU, mV, mW);
                mOrientationDirty = false;
            }

            void _detUVW() {
                glm::mat3 trans(glm::transpose(mOrientation));
                mU = glm::normalize(trans[0]);
                mV = glm::normalize(trans[1]);
                mW = glm::normalize(trans[2]);
            }

            mutable glm::mat3 mOrientation;
            mutable bool mOrientationDirty;

    };

}