#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

#include "WaterComponent.hpp"
#include "WaterMeshComponent.hpp"

using namespace neo;

class WaterShader : public Shader {

public:

    Texture* waveTextureDirection;
    Texture* waveTextureData;

    glm::vec4 normalMapScrollDir = glm::vec4(0.25f, 0.25f, 0.25f, -0.25f);
    glm::vec2 normalMapScrollSpeed = glm::vec2(0.2f, 0.3f);

    Texture* waterNoise;

    float refractionFactor = 0.04f;
    float refractionHeightFactor = 2.5f;
    float refractionDistanceFactor = 15.0f;

    glm::vec3 refractionColor = glm::vec3(0.003f, 0.599f, 0.812f);
    glm::vec3 surfaceColor = glm::vec3(0.465f, 0.797f, 0.991f);

    float depthSofteningDistance = 0.5f;

    float reflectionPow = 3.f;
    glm::vec4 ssrSettings = glm::vec4(0.5f, 20.f, 10.f, 20.f);

    bool wireframe = false;

    WaterShader(const std::string &vert, const std::string &frag, const std::string& control, const std::string& eval) :
        Shader("Water Shader") {
        _attachStage(ShaderStage::VERTEX, vert);
        _attachStage(ShaderStage::FRAGMENT, frag);
        _attachStage(ShaderStage::TESSELLATION_CONTROL, control);
        _attachStage(ShaderStage::TESSELLATION_EVAL, eval);
        init();

        TextureFormat directionFormat;
        directionFormat.inputFormat = GL_RGB16F;
        directionFormat.format = GL_RGB;
        directionFormat.filter = GL_LINEAR;
        directionFormat.mode = GL_CLAMP_TO_EDGE;
        directionFormat.type = GL_FLOAT;
        waveTextureDirection = Library::createEmptyTexture<Texture2D>("waveDirection", directionFormat);

        TextureFormat dataFormat;
        dataFormat.inputFormat = GL_RGBA16F;
        dataFormat.format = GL_RGBA;
        dataFormat.filter = GL_LINEAR;
        dataFormat.mode = GL_CLAMP_TO_EDGE;
        dataFormat.type = GL_FLOAT;
        waveTextureData = Library::createEmptyTexture<Texture2D>("wavedata", dataFormat);

        TextureFormat noiseFormat;
        noiseFormat.inputFormat = GL_R16F;
        noiseFormat.format = GL_RED;
        noiseFormat.filter = GL_LINEAR;
        noiseFormat.mode = GL_REPEAT;
        noiseFormat.type = GL_FLOAT;
        waterNoise = Library::createEmptyTexture<Texture2D>("waterNoise", noiseFormat);
        std::vector<float> data;
        const int noisesize = 256;
        data.resize(noisesize * noisesize * 1);
        for (int i = 0; i < noisesize * noisesize * 1; i++) {
            data[i] = Util::genRandom(-1.f, 1.f);
        }
        waterNoise->update(glm::uvec2(noisesize, noisesize), data.data());
    }

    virtual void render() override {
        bind();

        CHECK_GL(glDisable(GL_BLEND));
        if (wireframe) {
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        else {
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent, SpatialComponent>()) {
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());
            loadUniform("invP", glm::inverse(camera->get<CameraComponent>()->getProj()));
            loadUniform("invV", glm::inverse(camera->get<CameraComponent>()->getView()));
            loadUniform("camPos", camera->get<SpatialComponent>()->getPosition());
        }

        loadUniform("time", Util::getRunTime());

        if (auto light = Engine::getComponentTuple<LightComponent, SpatialComponent>()) {
            loadUniform("lightPos", light->get<SpatialComponent>()->getPosition());
        }

        loadTexture("waveDir", *waveTextureDirection);
        loadTexture("waveData", *waveTextureData);

        loadTexture("WaterNormalMap1", *Library::getTexture("water1.png", { GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT }));
        loadTexture("WaterNormalMap2", *Library::getTexture("water2.png", { GL_RGBA, GL_RGBA, GL_LINEAR, GL_REPEAT }));

        loadUniform("normalMapScrollDir", normalMapScrollDir);
        loadUniform("normalMapScrollSpeed", normalMapScrollSpeed);

        loadTexture("waterNoise", *waterNoise);

        loadUniform("refractionDistortionFactor", refractionFactor);
        loadUniform("refractionHeightFactor", refractionHeightFactor);
        loadUniform("refractionDistanceFactor", refractionDistanceFactor);

        loadTexture("gWorld", *Library::getFBO("gbuffer")->mTextures[0]);
        loadTexture("gNormal", *Library::getFBO("gbuffer")->mTextures[1]);
        loadTexture("gDiffuse", *Library::getFBO("gbuffer")->mTextures[2]);
        loadTexture("gDepth", *Library::getFBO("gbuffer")->mTextures[3]);

        loadUniform("reflectionPow", reflectionPow);
        loadUniform("ssrSettings", ssrSettings);

        loadUniform("surfaceColor", surfaceColor);
        loadUniform("refractionColor", refractionColor);

        loadUniform("depthSofteningDistance", depthSofteningDistance);

        if (auto skybox = Engine::getComponentTuple<renderable::SkyboxComponent, CubeMapComponent>()) {
            loadTexture("cubeMap", skybox->get<CubeMapComponent>()->mTexture);
        }

        if (auto renderable = Engine::getComponentTuple<WaterComponent, WaterMeshComponent, SpatialComponent>()) {
            if (renderable->get<WaterComponent>()->mDirtyWave) {
                std::vector<float> rawDirectionData;
                rawDirectionData.resize(renderable->get<WaterComponent>()->mWaveData.size() * 3);
                std::vector<float> rawWaveData;
                rawWaveData.resize(renderable->get<WaterComponent>()->mWaveData.size() * 4);
                for (int i = 0; i < renderable->get<WaterComponent>()->mWaveData.size(); i++) {
                    rawDirectionData[3 * i + 0] = renderable->get<WaterComponent>()->mWaveData[i].direction.x;
                    rawDirectionData[3 * i + 1] = renderable->get<WaterComponent>()->mWaveData[i].direction.y;
                    rawDirectionData[3 * i + 2] = renderable->get<WaterComponent>()->mWaveData[i].direction.z;
                    waveTextureDirection->update(glm::uvec2(renderable->get<WaterComponent>()->mWaveData.size(), 1), rawDirectionData.data());

                    rawWaveData[4 * i + 0] = renderable->get<WaterComponent>()->mWaveData[i].amplitude;
                    rawWaveData[4 * i + 1] = renderable->get<WaterComponent>()->mWaveData[i].speed;
                    rawWaveData[4 * i + 2] = renderable->get<WaterComponent>()->mWaveData[i].steepness;
                    rawWaveData[4 * i + 3] = renderable->get<WaterComponent>()->mWaveData[i].waveLength;
                    waveTextureData->update(glm::uvec2(renderable->get<WaterComponent>()->mWaveData.size(), 1), rawWaveData.data());
                }
                renderable->get<WaterComponent>()->mDirtyWave = false;
            }

            loadUniform("tessFactor", renderable->get<WaterMeshComponent>()->mTessFactor);
            loadUniform("tessDistance", renderable->get<WaterMeshComponent>()->mTessDistance);
            loadUniform("dampeningFactor", renderable->get<WaterComponent>()->mDampening);

            loadUniform("numWaves", static_cast<int>(renderable->get<WaterComponent>()->mWaveData.size()));

            loadUniform("reflectanceFactor", renderable->get<WaterComponent>()->mReflectanceFactor);
            loadUniform("roughness", renderable->get<WaterComponent>()->mRoughness);
            loadUniform("specIntensity", renderable->get<WaterComponent>()->mSpecIntensity);

            loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());
            loadUniform("N", renderable->get<SpatialComponent>()->getNormalMatrix());

            CHECK_GL(glPatchParameteri(GL_PATCH_VERTICES, 3));
            renderable->get<WaterMeshComponent>()->getMesh().draw();
        }

        unbind();
    }

    virtual void imguiEditor() override {
        ImGui::Checkbox("Wireframe", &wireframe);

        ImGui::SliderFloat2("NormalMap1 scroll dir", &normalMapScrollDir[0], 0.f, 1.f);
        ImGui::SliderFloat("NormalMap1 scroll speed", &normalMapScrollSpeed[0], 0.f, 1.f);
        ImGui::SliderFloat2("NormalMap2 scroll dir", &normalMapScrollDir[2], 0.f, 1.f);
        ImGui::SliderFloat("NormalMap2 scroll speed", &normalMapScrollSpeed[1], 0.f, 1.f);

        ImGui::SliderFloat("Refraction Factor", &refractionFactor, 0.f, 1.f);
        ImGui::SliderFloat("Refraction Height", &refractionHeightFactor, 0.f, 50.f);
        ImGui::SliderFloat("Refraction Distance", &refractionDistanceFactor, 0.f, 50.f);
        ImGui::ColorEdit3("Refraction Color", &refractionColor[0]);

        ImGui::ColorEdit3("Surface Color", &surfaceColor[0]);
        ImGui::SliderFloat("Reflection Pow", &reflectionPow, 0.f, 100.f);
        ImGui::SliderFloat4("SSR", &ssrSettings[0], 0.f, 100.f);

        ImGui::SliderFloat("Depth dist", &depthSofteningDistance, 0.f, 50.f);
    }
};