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

    class PhongShader : public Shader {

    public:

        PhongShader() :
            Shader("Phong Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 1) in vec3 vertNor;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                uniform mat3 N;
                out vec4 fragPos;
                out vec3 fragNor;
                out vec2 fragTex;
                void main() {
                    fragPos = M * vec4(vertPos, 1.0);
                    fragNor = N * vertNor;
                    fragTex = vertTex;
                    gl_Position = P * V * fragPos;
                })",
                R"(
                #include "phong.glsl"
                #include "alphaDiscard.glsl"

                in vec4 fragPos;
                in vec3 fragNor;
                in vec2 fragTex;
                uniform sampler2D diffuseMap;
                uniform vec3 ambientColor;
                uniform vec3 diffuseColor;
                uniform vec3 specularColor;
                uniform float shine;
                uniform vec3 camPos;
                uniform vec3 lightPos;
                uniform vec3 lightCol;
                uniform vec3 lightAtt;

                #include "writevoxel.glsl"

                out vec4 color;
                void main() {
                    vec4 albedo = texture(diffuseMap, fragTex);
                    albedo.rgb += diffuseColor;
                    alphaDiscard(albedo.a);
                    color.rgb = albedo.rgb * ambientColor + 
                                getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
                    color.a = albedo.a;
                    writevoxel(color);
                })")
        {
            TextureFormat format = { GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE };
            auto fbo = Library::createFBO("downscalebackbuffer");
            fbo->attachColorTexture({ 1, 1 }, format);
            fbo->attachDepthTexture({ 1, 1 }, GL_LINEAR, GL_CLAMP_TO_EDGE);
            fbo->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage>([&](const Message& msg) {
                glm::ivec2 frameSize = (static_cast<const FrameSizeMessage&>(msg)).mSize;
                Library::getFBO("downscalebackbuffer")->resize(frameSize / 2);
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
            if (const auto& cameraOpt = ecs.getSingleView<VolumeWriteCameraComponent, PerspectiveCameraComponent, SpatialComponent>()) {
                auto&& [_, __, camera, cameraSpatial] = *cameraOpt;
                loadUniform("P", camera.getProj());
                loadUniform("camNear", camera.getNearFar().x);
                loadUniform("camFar", camera.getNearFar().y);
                loadUniform("V", cameraSpatial.getView());
                loadUniform("camPos", cameraSpatial.getPosition());
            }

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

                loadUniform("bufferSize", glm::vec2(fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));
            }

            for (const auto&& [entity, renderable, mesh, spatial] : ecs.getView<renderable::PhongRenderable, MeshComponent, SpatialComponent>().each()) {
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
