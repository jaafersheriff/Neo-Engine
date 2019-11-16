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
        loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
        }

        if (auto skybox = Engine::getComponentTuple<renderable::SkyboxComponent, CubeMapComponent>()) {
            loadUniform("cubeMap", skybox->get<CubeMapComponent>()->mTexture.mTextureID);
        }

        CHECK_GL(glDisable(GL_CULL_FACE));
        if (mWireframe) {
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        for (auto& metaball : Engine::getComponentTuples<MetaballsMeshComponent, MeshComponent, SpatialComponent>()) {

            loadUniform("wireframe", mWireframe);
            loadUniform("M", glm::mat4(1.f)); // metaball->get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", glm::mat3(1.f)); // metaball->get<SpatialComponent>()->getNormalMatrix());

            /* DRAW */
            metaball->get<MeshComponent>()->getMesh().draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &mWireframe);
    }
};
