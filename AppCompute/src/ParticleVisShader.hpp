#pragma once

#include "Engine.hpp"
#include "ParticleMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class ParticleVisShader : public Shader {

public:

    ParticleVisShader(const std::string &vert, const std::string& frag, const std::string &geom) :
        Shader("ParticleVis Shader", vert, frag, geom)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        for (auto& model : Engine::getComponentTuples<ParticleMeshComponent, SpatialComponent>()) {
            // CHECK_GL(glEnable(GL_BLEND));
            // CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glDisable(GL_CULL_FACE));

            loadUniform("M", model->get<SpatialComponent>()->getModelMatrix());

        CHECK_GL(glBindVertexArray(model->get<ParticleMeshComponent>()->mMesh->mVAOID));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, model->get<ParticleMeshComponent>()->mMesh->getVBO(VertexType::Position).vboID));


            /* DRAW */
            model->get<ParticleMeshComponent>()->mMesh->draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
    }
};
