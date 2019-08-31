#pragma once

#include "Shader/Shader.hpp"

namespace neo {

    class PostProcessShader : public Shader {
    public:
        PostProcessShader(const std::string &name, const std::string &frag) :
            Shader(name, 
                R"(
                layout (location = 0) in vec3 vertPos;
                layout (location = 2) in vec2 vertTex;
                out vec2 fragTex;
                void main() { gl_Position = vec4(2 * vertPos, 1); fragTex = vertTex; })", 
                frag)
        {}
    };

}
