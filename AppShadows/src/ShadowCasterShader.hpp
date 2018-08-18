#pragma once

#include "Shader/Shader.hpp"
#include "System/RenderSystem/Framebuffer.hpp"
#include "System/RenderSystem/RenderSystem.hpp"

using namespace neo;

class ShadowCasterShader : public Shader {

    public:

        Framebuffer * depthFBO;
        Texture * depthTexture;

        ShadowCasterShader(RenderSystem &rSystem, const std::string &vert, const std::string &frag) :
            Shader("Shadow Caster", rSystem, vert, frag) {

            /* Init shadow map */
            depthTexture = new Texture;
            depthTexture->width = 1024;
            depthTexture->height = 1024;
            depthTexture->components = 1;
            depthTexture->upload(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_NEAREST, GL_REPEAT);

            Framebuffer *depthFBO = rSystem.createFBO("depthMap");
            depthFBO->attachDepthTexture(*depthTexture);
            depthFBO->disableDraw();
            depthFBO->disableRead();
        }

        virtual ~ShadowCasterShader() {
            delete depthTexture;
        }
};