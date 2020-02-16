#pragma once

#include "Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

using namespace neo;

class MyPhongShader : public Shader {

public:

    MyPhongShader(const std::string& vert, const std::string& frag) :
        Shader("MyPhong Shader", vert, frag)
    { }

    virtual void render() override {
        bind();

        /* Load PV */
        auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>();
        NEO_ASSERT(camera, "No main camera exists");
        loadUniform("P", camera->get<CameraComponent>()->getProj());
        loadUniform("V", camera->get<CameraComponent>()->getView());

        loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());

        /* Load light */
        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
            loadUniform("lightCol", light->get<LightComponent>()->mColor);
            loadUniform("lightAtt", light->get<LightComponent>()->mAttenuation);
        }

        const auto& cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

        for (auto& renderable : Engine::getComponentTuples<renderable::PhongRenderable, ParentComponent>()) {
            auto parent = renderable->get<ParentComponent>();
            glm::mat4 pM(1.f);
            glm::mat3 pN(1.f);
            if (auto spatial = parent->getGameObject().getComponentByType<SpatialComponent>()) {
                pM = spatial->getModelMatrix();
                pN = spatial->getNormalMatrix();
            }

            for (auto& child : renderable->get<ParentComponent>()->childrenObjects) {
                if (auto mesh = child->getComponentByType<MeshComponent>()) {
                    RenderDescription description;
                    description.mesh = &mesh->getMesh();

                    glm::mat4 M = pM;
                    glm::mat3 N = pN;
                    if (auto spatial = child->getComponentByType<SpatialComponent>()) {
                        M = M * spatial->getModelMatrix();
                        N = N * spatial->getNormalMatrix();
                    }
                    description.M = M;
                    description.N = N;

                    if (auto ambientMap = child->getComponentByType<AmbientMapComponent>()) {
                        description.ambientMap = &ambientMap->mTexture;
                    }
                    if (auto diffuseMap = child->getComponentByType<DiffuseMapComponent>()) {
                        description.diffuseMap = &diffuseMap->mTexture;
                    }
                    if (auto specularMap = child->getComponentByType<SpecularMapComponent>()) {
                        description.specularMap = &specularMap->mTexture;
                    }
                    if (auto normalMap = child->getComponentByType<NormalMapComponent>()) {
                        description.normalMap = &normalMap->mTexture;
                    }

                    Material material;
                    MaterialComponent* mc = parent->getGameObject().getComponentByType<MaterialComponent>();
                    if (auto m = child->getComponentByType<MaterialComponent>()) {
                        mc = m;
                    }
                    if (mc) {
                        material = mc->mMaterial;
                    }
                    description.material = material;

                    _render(description);
                }
            }
        }

        for (auto& renderable : Engine::getComponentTuples<renderable::PhongRenderable, MeshComponent, SpatialComponent>()) {
            auto renderableSpatial = renderable->get<SpatialComponent>();

            // VFC
            if (cameraFrustum) {
                MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                    float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                    if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                        continue;
                    }
                }
            }

            RenderDescription description;
            description.M = renderableSpatial->getModelMatrix();
            description.N = renderableSpatial->getNormalMatrix();

            if (auto ambientMap = renderable->mGameObject.getComponentByType<AmbientMapComponent>()) {
                description.ambientMap = &ambientMap->mTexture;
            }
            if (auto diffuseMap = renderable->mGameObject.getComponentByType<DiffuseMapComponent>()) {
                description.diffuseMap = &diffuseMap->mTexture;
            }
            if (auto specularMap = renderable->mGameObject.getComponentByType<SpecularMapComponent>()) {
                description.specularMap = &specularMap->mTexture;
            }
            if (auto normalMap = renderable->mGameObject.getComponentByType<NormalMapComponent>()) {
                description.normalMap = &normalMap->mTexture;
            }

            /* Bind material */
            Material material;
            if (auto matComp = renderable->mGameObject.getComponentByType<MaterialComponent>()) {
                material = matComp->mMaterial;
            }

            _render(description);
        }
    }

private:

    struct RenderDescription {
        glm::mat4 M;
        glm::mat3 N;
        Material material;
        Texture* ambientMap;
        Texture* diffuseMap;
        Texture* specularMap;
        Texture* normalMap;
        Mesh* mesh;
    };

    void _render(RenderDescription& description) {
        loadUniform("M", description.M);
        loadUniform("N", description.N);
        loadUniform("ambient", description.material.ambient);
        loadUniform("diffuse", description.material.diffuse);
        loadUniform("specular", description.material.specular);
        loadUniform("shine", description.material.shininess);
        loadTexture("ambientMap", description.ambientMap ? *description.ambientMap : *Library::getTexture("white"));
        loadTexture("diffuseMap", description.diffuseMap ? *description.diffuseMap : *Library::getTexture("white"));
        loadTexture("specularMap", description.specularMap ? *description.specularMap : *Library::getTexture("white"));
        loadTexture("alphaMap", description.normalMap ? *description.normalMap : *Library::getTexture("black"));

        description.mesh->draw();
    }
};
