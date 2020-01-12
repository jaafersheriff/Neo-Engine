#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class AlphaTestShader : public Shader {

    public:

        AlphaTestShader() :
            Shader("AlphaTest Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                out vec2 fragTex;
                void main() {
                    fragTex = vertTex;
                    gl_Position = P * V * M * vec4(vertPos, 1.0);
                })",
                R"(
                #include "alphaDiscard.glsl"
                in vec2 fragTex;
                uniform sampler2D diffuseMap;
                out vec4 color;
                void main() {
                    vec4 albedo = texture(diffuseMap, fragTex);
                    alphaDiscard(albedo.a);
                    color = albedo;
                })")
        {
            // Create alphart 
            auto alphart = Library::getFBO("alphart");
            alphart->generate();

            // Format for color buffers
            TextureFormat format = { GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT };
            alphart->attachColorTexture(Window::getFrameSize(), format); // diffuse
            alphart->attachDepthTexture(Library::getTexture("shareddepth"));
            alphart->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("alphart")->resize(frameSize);
            });

        }

        virtual void render() override {
            auto fbo = Library::getFBO("alphart");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            bind();

            /* Load PV */
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (auto& renderable : Engine::getComponentTuples<renderable::AlphaTestRenderable, MeshComponent, SpatialComponent>()) {
                auto spatial = renderable->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());

                /* Bind texture */
                if (const auto diffuseMap = renderable->mGameObject.getComponentByType<DiffuseMapComponent>()) {
                    loadTexture("diffuseMap", diffuseMap->mTexture);
                }

                /* DRAW */
                renderable->get<MeshComponent>()->getMesh().draw();
            }

            unbind();
        }
    };
}
