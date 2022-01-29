#pragma once

#include "Engine.hpp"
#include "MetaballsMeshComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

using namespace neo;

class MetaballsShader : public Shader {

public:

    bool mWireframe = false;

    MetaballsShader(const std::string &vert, const std::string &frag) :
        Shader("Metaballs Shader", vert, frag)
    {}

    virtual void render() override {
        bind();

        if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>()) {
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
            loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());
        }
        else {
            return;
        }

        if (auto skybox = Engine::getSingleComponent<renderable::SkyboxComponent>()) {
            loadTexture("cubeMap", skybox->mCubeMap);
        }

        CHECK_GL(glDisable(GL_CULL_FACE));
        if (mWireframe) {
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        for (auto& metaball : Engine::getComponentTuples<MetaballsMeshComponent, SpatialComponent>()) {

            loadUniform("wireframe", mWireframe);
            loadUniform("M", metaball->get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", metaball->get<SpatialComponent>()->getNormalMatrix());

            /* DRAW */
            metaball->get<MetaballsMeshComponent>()->mMesh->draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &mWireframe);
    }
};
