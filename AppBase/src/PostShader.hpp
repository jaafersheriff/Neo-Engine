#pragma once

#include "Shader/PostProcessShader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "GLObjects/Framebuffer.hpp"

using namespace neo;

class PostShader : public PostProcessShader {

    public:

        PostShader(const std::string& frag) :
            PostProcessShader("Post Shader", frag)
        {}

        virtual void render() override {
            loadTexture("dofDown", *Library::getFBO("dofdown")->mTextures[0]);
            loadTexture("dofBlur", *Library::getFBO("dofblur")->mTextures[0]);
        }

        virtual void imguiEditor() override {
        }
};
