#pragma once

#include "Engine/Engine.hpp"
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

    virtual void render(const ECS& ecs) override {
        bind();

        if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>()) {
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
            loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());
        }
        else {
            return;
        }

        if (auto skybox = ecs.getSingleComponent<renderable::SkyboxComponent>()) {
            loadTexture("cubeMap", skybox->mCubeMap);
        }

        CHECK_GL(glDisable(GL_CULL_FACE));
        if (mWireframe) {
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        for (auto& metaball : ecs.getComponentTuples<MetaballsMeshComponent, SpatialComponent>()) {

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
