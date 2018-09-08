#pragma once

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class PostProcessShader : public Shader {
    public: 
        PostProcessShader(const std::string &frag) :
            Shader("PostProcessShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};