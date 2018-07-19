#include "CameraControllerComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"

#include "Util/Util.hpp"

namespace neo {

    CameraControllerComponent::CameraControllerComponent(GameObject &go, float lookSpeed, float moveSpeed) :
        Component(go),
        spatial(nullptr),
        theta(0.f),
        phi(Util::PI() * 0.5f),
        lookSpeed(lookSpeed),
        moveSpeed(moveSpeed) {
        setButtons(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_R);
    }

    void CameraControllerComponent::setButtons(int f, int b, int l, int r, int u, int d) {
        forwardButton  = f;
        backwardButton = b;
        rightButton    = l;
        leftButton     = r;
        upButton       = u;
        downButton     = d;
    }

    void CameraControllerComponent::init() {
        assert(spatial = getGameObject().getSpatial());
    }

    void CameraControllerComponent::update(float dt) {
        if (Mouse::dx || Mouse::dy) {
            theta -= float(Mouse::dx) * lookSpeed;
            phi   += float(Mouse::dy) * lookSpeed;
            updateSpatialOrientation();
        }

        int forward(Keyboard::isKeyPressed(forwardButton));
        int backward(Keyboard::isKeyPressed(backwardButton));
        int right(Keyboard::isKeyPressed(rightButton));
        int left(Keyboard::isKeyPressed(leftButton));
        int up(Keyboard::isKeyPressed(upButton));
        int down(Keyboard::isKeyPressed(downButton));

        glm::vec3 dir(
            float(right - left),
            float(up - down),
            float(backward - forward)
        );

        if (dir != glm::vec3()) {
            dir = glm::normalize(dir);
            dir = glm::normalize(spatial->getU() * dir.x + glm::vec3(0.f, 1.f, 0.f) * dir.y + spatial->getW() * dir.z);
            spatial->move(dir * moveSpeed * dt);
        }
    }

    void CameraControllerComponent::setOrientation(float theta, float phi) {
        this->theta = theta;
        this->phi = phi;
        updateSpatialOrientation();
    }

    void CameraControllerComponent::updateSpatialOrientation() {
        if (theta > Util::PI()) {
            theta = std::fmod(theta, Util::PI()) - Util::PI();
        }
        else if (theta < -Util::PI()) {
            theta = Util::PI() - std::fmod(-theta, Util::PI());
        }

        /* phi [0.f, pi] */
        phi = glm::max(glm::min(phi, Util::PI()), 0.f);

        glm::vec3 w(-Util::sphericalToCartesian(1.0f, theta, phi));
        w = glm::vec3(-w.y, w.z, -w.x); // one of the many reasons I like z to be up
        glm::vec3 v(Util::sphericalToCartesian(1.0f, theta, phi - Util::PI() * 0.5f));
        v = glm::vec3(-v.y, v.z, -v.x);
        glm::vec3 u(glm::cross(v, w));
        spatial->setUVW(u, v, w);
    }
}