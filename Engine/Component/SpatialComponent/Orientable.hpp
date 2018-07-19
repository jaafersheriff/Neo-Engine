#pragma once

#include <glm/glm.hpp>

namespace neo {

    class Orientable {

        public:
            Orientable() :
                u(1.f, 0.f, 0.f),
                v(0.f, 1.f, 0.f),
                w(0.f, 0.f, 1.f),
                orientation(),
                orientationDirty(false)
            {}

            Orientable(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) :
                u(u),
                v(v),
                w(w),
                orientation(),
                orientationDirty(true)
            {}

            virtual ~Orientable() = default;

            /* Update */
            virtual void rotate(const glm::mat3 & mat) {
                orientation = mat * orientation;
                orientationDirty = false;
                detUVW();
            }

            /* Setters */
            virtual void setOrientation(const glm::mat3 & o) {
                orientation = o;
                orientationDirty = false;
                detUVW();
            }
            virtual void setUVW(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) {
                this->u = glm::normalize(u);
                this->v = glm::normalize(v);
                this->w = glm::normalize(w);
                orientationDirty = true;
            }

            /* Getters */
            virtual const glm::vec3 & getU() const { return u; }
            virtual const glm::vec3 & getV() const { return v; }
            virtual const glm::vec3 & getW() const { return w; }
            virtual const glm::mat3 & getOrientation() const {
                if (orientationDirty) {
                    detOrientation();
                }
                return orientation;
            }

        private:    
            void detOrientation() const {
                orientation = glm::mat3(u, v, w);
                orientationDirty = false;
            }

            void detUVW() {
                glm::mat3 trans(glm::transpose(orientation));
                u = glm::normalize(trans[0]);
                v = glm::normalize(trans[1]);
                w = glm::normalize(trans[2]);
            }

            glm::vec3 u, v, w;
            mutable glm::mat3 orientation;
            mutable bool orientationDirty;

    };

}