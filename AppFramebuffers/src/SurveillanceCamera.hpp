#pragma once

#include "Component/CameraComponent/CameraComponent.hpp"
#include "GLHelper/Framebuffer.hpp"

#include "Messaging/Messenger.hpp"

using namespace neo;

class SurveillanceCamera : public CameraComponent {
    public:

        Framebuffer * fbo;
        Texture2D * colorBuffer;
        Texture2D * depthBuffer;

        SurveillanceCamera(GameObject *go, float near, float far) :
            CameraComponent(go, -10.f, 10.f, -10.f, 10.f, near, far) {
            colorBuffer =  new Texture2D;
            colorBuffer->width = Window::getFrameSize().x;
            colorBuffer->height = Window::getFrameSize().y;
            colorBuffer->components = 4;
            colorBuffer->upload(GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT);

            depthBuffer = new Texture2D;
            depthBuffer->width = Window::getFrameSize().x;
            depthBuffer->height = Window::getFrameSize().y;
            depthBuffer->components = 1;
            depthBuffer->upload(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_NEAREST, GL_REPEAT);

            fbo = new Framebuffer;
            fbo->generate();
            fbo->attachColorTexture(*colorBuffer);
            fbo->attachDepthTexture(*depthBuffer);
        }

        virtual void update(float dt) override {
            float scale = gameObject->getSpatial()->getScale().x;
            glm::vec2 bounds(-scale, scale);
            setOrthoBounds(bounds, bounds);
        }

        virtual void kill() override {
            delete colorBuffer;
            delete depthBuffer;
            delete fbo;
        }
};