#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"

using namespace neo;

class LightShader : public Shader {

    public:

        LightShader(const std::string &vert, const std::string &frag) :
            Shader("LightShader", vert, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {

            bind();

            auto mesh = Loader::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

            mesh->draw();

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
    }
};