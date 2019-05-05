#pragma once

#include "Engine.hpp"
#include "SnowComponent.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"
#include "Renderer/Renderer.hpp"

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
        loadUniform("snowAngle", snow->snowAngle);
        loadUniform("snowColor", snow->snowColor);
        loadUniform("snowSize", snow->snowSize);
        loadUniform("height", snow->height);
        loadUniform("rimColor", snow->rimColor);
        loadUniform("rimPower", snow->rimPower);

        /* Load Camera */
        loadUniform("P", camera.getProj());
        loadUniform("V", camera.getView());
        loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

        /* Load light */
        auto lights = Engine::getComponents<LightComponent>();
        if (lights.size()) {
            loadUniform("lightPos", lights.at(0)->getGameObject().getSpatial()->getPosition());
            loadUniform("lightCol", lights.at(0)->mColor);
            loadUniform("lightAtt", lights.at(0)->mAttenuation);
        }

        for (auto& model : Engine::getComponents<MeshComponent>()) {
            loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
            loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

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

        CHECK_GL(glBindVertexArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        unbind();
    }
};
