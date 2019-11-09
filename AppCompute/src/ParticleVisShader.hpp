#pragma once

#include "Engine.hpp"
#include "ParticleMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class ParticleVisShader : public Shader {

public:

    ParticleVisShader(const std::string &vert, const std::string &frag) :
        Shader("ParticleVis Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        for (auto& model : Engine::getComponentTuples<ParticleMeshComponent, SpatialComponent>()) {
            CHECK_GL(glEnable(GL_BLEND));
            CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glDisable(GL_CULL_FACE));

            loadUniform("M", model->get<SpatialComponent>()->getModelMatrix());

            /* Bind mesh */
            CHECK_GL(glBindVertexArray(model->get<ParticleMeshComponent>()->mMesh->mVAOID));
            CHECK_GL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, model->get<ParticleMeshComponent>()->mMesh->getVBO(VertexType::Position).vboID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->get<ParticleMeshComponent>()->mMesh->mElementVBO->vboID));

            /* DRAW */
            model->get<ParticleMeshComponent>()->mMesh->draw(model->get<ParticleMeshComponent>()->mNumVerts * 6);
        }

        unbind();
    }

    virtual void imguiEditor() override {
    }
};
