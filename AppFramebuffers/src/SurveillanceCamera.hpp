#pragma once

#include "Component/CameraComponent/CameraComponent.hpp"
#include "System/RenderSystem/Framebuffer.hpp"

using namespace neo;

class SurveillanceCamera : public CameraComponent {
    public:

        Framebuffer * fbo;
        Texture * fboTex;

        SurveillanceCamera(GameObject *go, float fov, float near, float far) :
            CameraComponent(go, fov, near, far) {
            fboTex = new Texture;
            fboTex->width = Window::getFrameSize().x;
            fboTex->height = Window::getFrameSize().y;
            fboTex->components = 3;
            fboTex->upload(GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT);

            fbo = new Framebuffer;
            fbo->generate();
            fbo->attachColorTexture(*fboTex);
        }

        virtual void kill() override {
            delete fboTex;
            delete fbo;
        }
};