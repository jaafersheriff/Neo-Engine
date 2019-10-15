#pragma once

#include "Engine.hpp"
#include "SnowComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

using namespace neo;

class SnowShader : public Shader {

public:

    SnowShader(const std::string &vert, const std::string &frag) :
        Shader("Snow Shader", vert, frag)
    {}

    virtual void render(const CameraComponent &camera) override {
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
        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());
        loadUniform("camPos", camera.getGameObject().getComponentByType<SpatialComponent>()->getPosition());

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

            /* Bind mesh */
            const Mesh & mesh(model->getMesh());
            CHECK_GL(glBindVertexArray(mesh.mVAOID));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

            /* Bind material */
            if (auto material = model->getGameObject().getComponentByType<MaterialComponent>()) {
                loadUniform("ambient", material->mAmbient);
                loadUniform("diffuseColor", material->mDiffuse);
                loadUniform("specularColor", material->mSpecular);
                loadUniform("shine", material->mShine);
            }

            /* DRAW */
            mesh.draw();
        }

        unbind();
    }
};
