#pragma once

#include "Engine.hpp"
#include "MetaballsComponent.hpp"

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
        loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
        }

        if (auto skybox = Engine::getComponentTuple<renderable::SkyboxComponent, CubeMapComponent>()) {
            loadUniform("cubeMap", skybox->get<CubeMapComponent>()->mTexture->mTextureID);
        }

        for (auto& metaball : Engine::getComponentTuples<MetaballsComponent, MeshComponent, SpatialComponent>()) {
            CHECK_GL(glDisable(GL_CULL_FACE));
            if (mWireframe) {
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            }

            loadUniform("wireframe", mWireframe);
            loadUniform("M", metaball.get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", metaball.get<SpatialComponent>()->getNormalMatrix());

            /* Bind mesh */
            auto& mesh = metaball.get<MeshComponent>()->getMesh();
            CHECK_GL(glBindVertexArray(mesh.mVAOID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.mVertexBufferID));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.mNormalBufferID));

            /* DRAW */
            mesh.draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &mWireframe);
    }
};
