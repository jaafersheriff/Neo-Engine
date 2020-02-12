#pragma once

#include "Engine.hpp"
#include "SnowComponent.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

using namespace neo;

class SnowShader : public Shader {

public:

    SnowShader(const std::string &vert, const std::string &frag) :
        Shader("Snow Shader", vert, frag)
    {}

    virtual void render() override {
        bind();

        /* Load snow */
        auto snow = Engine::getComponents<SnowComponent>()[0];
        loadUniform("snowAngle", snow->mSnowAngle);
        loadUniform("snowColor", snow->mSnowColor);
        loadUniform("snowSize", snow->mSnowSize);
        loadUniform("height", snow->mHeight);
        loadUniform("rimColor", snow->mRimColor);
        loadUniform("rimPower", snow->mRimPower);

        /* Load Camera */
        if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>()) {
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
            loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());
        }

        /* Load light */
        auto lights = Engine::getComponents<LightComponent>();
        if (lights.size()) {
            loadUniform("lightPos", lights.at(0)->getGameObject().getComponentByType<SpatialComponent>()->getPosition());
            loadUniform("lightCol", lights.at(0)->mColor);
            loadUniform("lightAtt", lights.at(0)->mAttenuation);
        }

        for (auto& model : Engine::getComponents<MeshComponent>()) {
            loadUniform("M", model->getGameObject().getComponentByType<SpatialComponent>()->getModelMatrix());
            loadUniform("N", model->getGameObject().getComponentByType<SpatialComponent>()->getNormalMatrix());

            /* Bind material */
            if (auto material = model->getGameObject().getComponentByType<MaterialComponent>()) {
                loadUniform("ambient", material->mAmbient);
                loadUniform("diffuseColor", material->mDiffuse);
                loadUniform("specularColor", material->mSpecular);
                loadUniform("shine", material->mShine);
            }

            /* DRAW */
            model->getMesh().draw();
        }

        unbind();
    }
};
