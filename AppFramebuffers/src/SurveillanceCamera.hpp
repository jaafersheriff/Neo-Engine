#pragma once

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Renderer/Renderer.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

// Wow this is hacky
class SurveillanceCamera : public CameraComponent {
    public:

        Framebuffer * fbo;

        SurveillanceCamera(GameObject *go, std::string name, float near, float far) :
            CameraComponent(go, -10.f, 10.f, -10.f, 10.f, near, far) {
            fbo = Library::getFBO(name);
            fbo->generate();
            fbo->attachColorTexture(Window::getFrameSize(), 4, TextureFormat{ GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT });
            fbo->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
        }

        virtual void kill() override {
            fbo->destroy();
        }
};