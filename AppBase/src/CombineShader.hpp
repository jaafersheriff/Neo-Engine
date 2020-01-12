#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class CombineShader : public PostProcessShader {

    public:

        CombineShader() :
            PostProcessShader("Combine Shader",     
                R"(
                    #include "postprocess.glsl"
                    uniform sampler2D phongrt;
                    uniform sampler2D alphart;
                    out vec4 color;
                    void main() {
                        vec4 phong = texture(phongrt, fragTex);
                        vec4 alpha = texture(alphart, fragTex);
                        color.rgb = phong.rgb + alpha.rgb;
                        color.a = 1.f;
                    })"
            ) 
        {}

        virtual void render() override {
            loadTexture("phongrt", *Library::getFBO("phongrt")->mTextures[0]);
            loadTexture("alphart", *Library::getFBO("alphart")->mTextures[0]);
        }
};
