#pragma once

#include "Engine.hpp"
#include "MetaballsMeshComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class MetaballsShader : public Shader {

public:

    bool mWireframe = false;

    MetaballsShader(const std::string &vert, const std::string &frag) :
        Shader("Metaballs Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
        bind();

        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());

        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
        }

        for (auto& model : Engine::getComponentTuples<MetaballsMeshComponent, SpatialComponent>()) {
            if (mWireframe) {
                CHECK_GL(glDisable(GL_CULL_FACE));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            }

            loadUniform("wf", mWireframe);   
            loadUniform("M", model.get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", model.get<SpatialComponent>()->getNormalMatrix());

            /* Bind mesh */
            CHECK_GL(glBindVertexArray(model.get<MetaballsMeshComponent>()->mMesh->mVAOID));
            CHECK_GL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, model.get<MetaballsMeshComponent>()->mMesh->mVertexBufferID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, model.get<MetaballsMeshComponent>()->mMesh->mNormalBufferID));

            /* DRAW */
            model.get<MetaballsMeshComponent>()->mMesh->draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &mWireframe);
    }
};
