#pragma once

#include "System/RenderSystem/Shader.hpp"

using namespace neo;

class TriangleShader : public Shader {

    public:
        TriangleShader(const std::string &res, const std::string &vert, const std::string &frag) :
            Shader(res, vert, frag)
        {}

        virtual void render() override {
            bind();
            unbind();
        }
};