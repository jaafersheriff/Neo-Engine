#pragma once

#include "Component/CameraComponent/CameraComponent.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class SurveillanceCamera : public CameraComponent {
    public:

        Framebuffer * fbo;

        SurveillanceCamera(GameObject *go, std::string name, float near, float far) :
            CameraComponent(go, -10.f, 10.f, -10.f, 10.f, near, far) {
            fbo = Loader::getFBO(name);
            fbo->generate();
            fbo->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT);
            fbo->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
        }

        virtual void kill() override {
            fbo->destroy();
        }
};