#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

using namespace neo;

class DOFShader : public PostProcessShader {

    public:

        DOFShader(const std::string &frag) :
            PostProcessShader("DOF Shader", frag) 
        {}

        virtual void render() override {
        }

        virtual void imguiEditor() override {
        }
};
