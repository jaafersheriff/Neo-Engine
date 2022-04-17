#pragma once

#include "ECS/ECS.hpp"

#include "Renderer/GLObjects/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderableComponent/PhongRenderable.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Library.hpp"
#include "ECS/Messaging/Messenger.hpp"
#include "ECS/Messaging/Message.hpp"

#include "VolumeWriteCameraComponent.hpp"

using namespace neo;

namespace Froxels {

    class VolumeWriteShader : public Shader {

    public:

        VolumeWriteShader(const std::string& vert, const std::string& frag) :
            Shader("VolumeWrite Shader", vert, frag) 
        {
            TextureFormat format = { GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE };
            auto fbo = Library::createFBO("downscalebackbuffer");
            fbo->attachColorTexture({ 1, 1 }, format);
            fbo->attachDepthTexture({ 1, 1 }, GL_LINEAR, GL_CLAMP_TO_EDGE);
            fbo->initDrawBuffers();

            Messenger::addReceiver<FrameSizeMessage>([&](const Message& msg) {
                glm::ivec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
                });
        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("downscalebackbuffer");
            fbo->bind();
            glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            bind();

            /* Load PV */
            const auto& cameraOpt = ecs.getSingleView<VolumeWriteCameraComponent, SpatialComponent>();
            if (cameraOpt) {
                auto&& [entity, __, cameraSpatial] = *cameraOpt;
                if (auto camera = ecs.getOneOfAs<CameraComponent, PerspectiveCameraComponent, OrthoCameraComponent>(entity)) {
                    loadUniform("P", camera->getProj());
                    loadUniform("camNear", camera->getNearFar().x);
                    loadUniform("camFar", camera->getNearFar().y);
                }
                loadUniform("V", cameraSpatial.getView());
                loadUniform("camPos", cameraSpatial.getPosition());
            }
            const auto& cameraFrustum = ecs.cGetComponent<FrustumComponent>(std::get<0>(*cameraOpt));

            /* Load light */
            if (const auto& lightTuple = ecs.getSingleView<LightComponent, SpatialComponent>()) {
                auto&& [_, light, spatial] = *lightTuple;
                loadUniform("lightCol", light.mColor);
                loadUniform("lightAtt", light.mAttenuation);
                loadUniform("lightPos", spatial.getPosition());
            }

            auto volume = Library::getTexture("Volume");
            {
                glClearTexImage(volume->mTextureID, 0, GL_RGBA, GL_FLOAT, 0);
                glBindImageTexture(0, volume->mTextureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                Library::getFBO("downscalebackbuffer")->resize(glm::uvec2(volume->mWidth, volume->mHeight));

                loadUniform("bufferSize", glm::vec2(fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));
            }

            for (const auto&& [entity, _, renderable, mesh, spatial] : ecs.getView<VolumeWriteComponent, renderable::PhongRenderable, MeshComponent, SpatialComponent>().each()) {
                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("PhongShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(entity)) {
                        if (!cameraFrustum->isInFrustum(spatial, *boundingBox)) {
                            continue;
                        }
                    }
                }

                loadUniform("M", spatial.getModelMatrix());
                loadUniform("N", spatial.getNormalMatrix());

                /* Bind texture */
                loadTexture("diffuseMap", *renderable.mDiffuseMap);

                /* Bind material */
                const Material& material = renderable.mMaterial;

                loadUniform("ambientColor", material.mAmbient);
                loadUniform("diffuseColor", material.mDiffuse);
                loadUniform("specularColor", material.mSpecular);
                loadUniform("shine", material.mShininess);

                /* DRAW */
                glFrontFace(GL_CW);
                mesh.mMesh->draw();
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
                glFrontFace(GL_CCW);
                mesh.mMesh->draw();
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
            }

            // TODO - generate mips manually in compute
            volume->generateMipMaps();
        }
    };
}
