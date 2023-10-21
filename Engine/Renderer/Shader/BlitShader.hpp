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
                #include "postprocess.glsl"
                out vec4 color;
                void main() {
                color = texture(inputFBO, fragTex);
                })")
        {}
    };
}