#pragma once

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

using namespace neo;

class RedShader : public Shader {
    public: 
        RedShader(const std::string &frag) :
            Shader("RedShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};

class GreenShader : public Shader {
    public: 
        GreenShader(const std::string &frag) :
            Shader("GreenShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};

class BlueShader : public Shader {
    public: 
        BlueShader(const std::string &frag) :
            Shader("BlueShader", MasterRenderer::POST_PROCESS_VERT_FILE, frag)
        {}
};