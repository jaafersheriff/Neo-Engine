#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "SunComponent.hpp"
#include "SunOccluderComponent.hpp"

using namespace neo;

class GodRayOccluderShader : public Shader {

    public:

        GodRayOccluderShader(const std::string &vert, const std::string &frag) :
            Shader("GodRayOccluder Shader", vert, frag) {
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("godray");
            fbo->bind();
            glm::ivec2 frameSize = Window::getFrameSize() / 2;
            CHECK_GL(glViewport(0, 0, frameSize.x, frameSize.y));

            bind();
            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (const auto& renderable : Engine::getComponents<SunOccluderComponent>()) {
                const auto& renderableMesh = renderable->getGameObject().getComponentByType<MeshComponent>();
                const auto& renderableSpatial = renderable->getGameObject().getComponentByType<SpatialComponent>();
                if (!renderableMesh || !renderableSpatial) {
                    continue;
                }

                // VFC
                if (const auto& boundingBox = renderable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    if (const auto& frustumPlanes = camera.getGameObject().getComponentByType<FrustumComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!frustumPlanes->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                /* Bind mesh */
                const Mesh & mesh(renderableMesh->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderableSpatial->getModelMatrix());

                /* Bind texture */
                if (auto diffuseMap = renderable->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    auto texture = (const Texture2D *)(diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("diffuseMap", texture->mTextureID);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
};
