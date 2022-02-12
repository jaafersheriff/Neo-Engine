#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/ECS.hpp"
#include "ParticleMeshComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

class ParticleVisShader : public Shader {

public:

    float mSpriteSize = 0.2f;
    glm::vec3 mSpriteColor = glm::vec3(0.67f, 1.f, 0.55f);

    ParticleVisShader(const std::string &vert, const std::string& frag, const std::string &geom) :
        Shader("ParticleVis Shader")
    {
        _attachStage(ShaderStage::VERTEX, vert);
        _attachStage(ShaderStage::FRAGMENT, frag);
        _attachStage(ShaderStage::GEOMETRY, geom);
        init();
    }

    virtual void render(const ECS& ecs) override {
        bind();

        if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>()) {
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
        }
        loadUniform("spriteSize", mSpriteSize);
        loadUniform("spriteColor", mSpriteColor);

        if (auto model = ecs.getSingleComponent<ParticleMeshComponent>()) {
            CHECK_GL(glEnable(GL_BLEND));
            CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glDisable(GL_CULL_FACE));

            loadUniform("M", model->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());

            /* DRAW */
            model->mMesh->draw();
        }
        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::SliderFloat("Sprite size", &mSpriteSize, 0.1f, 2.f);
        ImGui::ColorEdit3("Sprite color", &mSpriteColor[0]);
    }
};
