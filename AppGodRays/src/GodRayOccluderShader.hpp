#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"

using namespace neo;

class GodRayOccluderShader : public Shader {

    public:

        GodRayOccluderShader(const std::string &vert, const std::string &frag) :
            Shader("GodRayOccluder Shader", vert, frag) {
        }

        virtual void render() override {
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            if (!camera) {
                return;
            }

            auto fbo = Library::getFBO("godray");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();

            /* Load PV */
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderable : Engine::getComponentTuples<SunOccluderComponent, ParentComponent>()) {
                auto parent = renderable->get<ParentComponent>();
                glm::mat4 pM(1.f);
                if (auto spatial = parent->getGameObject().getComponentByType<SpatialComponent>()) {
                    pM = spatial->getModelMatrix();
                }

                for (auto& child : renderable->get<ParentComponent>()->childrenObjects) {
                    if (auto mesh = child->getComponentByType<MeshComponent>()) {

                        glm::mat4 M(pM);
                        if (auto spatial = child->getComponentByType<SpatialComponent>()) {
                            M = spatial->getModelMatrix() * pM;
                        }

                        _render(M, mesh->getMesh(), child->getComponentByType<DiffuseMapComponent>());
                    }
                }
            }

            for (auto& renderable : Engine::getComponentTuples<SunOccluderComponent, MeshComponent, SpatialComponent>()) {
                auto renderableMesh = renderable->get<MeshComponent>();
                auto renderableSpatial = renderable->get<SpatialComponent>();

                _render(renderableSpatial->getModelMatrix(), renderableMesh->getMesh(), renderable->mGameObject.getComponentByType<DiffuseMapComponent>());
            }
        }

    private:

        void _render(const glm::mat4& M, const Mesh& mesh, const DiffuseMapComponent* diffuseMap = nullptr) {
            loadUniform("M", M);

            Texture* texture = Library::getTexture("white");
            if (diffuseMap) {
                texture = &diffuseMap->mTexture;
            }

            loadTexture("diffuseMap", *texture);

            mesh.draw();
        }
};
