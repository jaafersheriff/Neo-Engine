#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"

using namespace neo;

class CombineShader : public Shader {

    public:

        CombineShader(const std::string &vert, const std::string &frag) :
            Shader("CombineShader", vert, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {

            bind();

            auto mesh = Loader::getMesh("quad");
            CHECK_GL(glBindVertexArray(mesh->vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

            /* Bind lights */
            // TODO : upload lights into a 1D texture 


            mesh->draw();

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
    }
};