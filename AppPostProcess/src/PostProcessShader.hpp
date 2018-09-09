#pragma once

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class InvertShader : public Shader {
    public: 
        InvertShader(const std::string &frag) :
            Shader("InvertShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};

class BWShader : public Shader {
    public: 
        BWShader(const std::string &frag) :
            Shader("BWShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};

class BlueShader : public Shader {
    public: 
        BlueShader(const std::string &frag) :
            Shader("BlueShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};

class DepthShader : public Shader {
    public: 
        DepthShader(const std::string &frag) :
            Shader("DepthShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};