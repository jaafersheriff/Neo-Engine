#pragma once

#include "Component/CameraComponent/OrthoCameraComponent.hpp"

using namespace neo;

// Wow this is hacky
class SurveillanceCamera : public OrthoCameraComponent {
    public:

        Framebuffer * fbo;

        SurveillanceCamera(GameObject *go, std::string name, float near, float far) :
            OrthoCameraComponent(go, near, far, -5.f, 5.f, -5.f, 5.f) {
            fbo = Library::getFBO(name);
            fbo->generate();
            fbo->attachColorTexture(Window::getFrameSize(), 4, TextureFormat{ GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT });
            fbo->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
        }

        virtual void kill() override {
            fbo->destroy();
        }
};