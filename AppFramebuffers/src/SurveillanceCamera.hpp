#pragma once

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Renderer/Renderer.hpp"

#include "Messaging/Messenger.hpp"

// Wow this is hacky
class SurveillanceCamera : public neo::CameraComponent {
    public:

        neo::Framebuffer * fbo;

        SurveillanceCamera(neo::GameObject *go, std::string name, float near, float far) :
            neo::CameraComponent(go, -10.f, 10.f, -10.f, 10.f, near, far) {
            fbo = neo::Library::getFBO(name);
            fbo->generate();
            fbo->attachColorTexture(neo::Window::getFrameSize(), 4, neo::TextureFormat{ GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT });
            fbo->attachDepthTexture(neo::Window::getFrameSize(), GL_NEAREST, GL_REPEAT);
        }

        virtual void kill() override {
            fbo->destroy();
        }
};