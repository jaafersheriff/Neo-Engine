#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"
#include "Engine/Engine.hpp"

namespace neo {

    class BlitShader : public PostProcessShader {

    public:

        BlitShader() :
            PostProcessShader("BlitShader", R"(
                in vec2 fragTex;
                uniform sampler2D inputTexture;
                out vec4 color;
                void main() {
                color = texture(inputTexture, fragTex);
                })")
        {}
    };
}