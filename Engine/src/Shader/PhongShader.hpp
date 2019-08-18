#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

namespace neo {

    class PhongShader : public Shader {

    public:
        float hscale = .1f;
        int layers = 10;
        float opacity = 0.055f;
        bool showNormals = false;

        PhongShader() :
            Shader("Phong Shader",
                R"(#version 330 core
                layout(location = 0) in vec3 vertPos;
                layout(location = 1) in vec3 vertNor;
                layout(location = 2) in vec2 vertTex;
                uniform mat4 P, V, M;
                uniform mat3 N;
                out vec4 fragPos;
                out vec3 fragNor;
                out vec2 fragTex;
                uniform vec3 lightPos;
                uniform vec3 camPos;

                out vec3 TanLightPos;
                out vec3 TanViewPos;
                out vec3 TanFragPos;
                out mat3 TBN;
                void main() {
                    fragPos = M * vec4(vertPos, 1.0);
                    fragNor = N * vertNor;
                    fragTex = vertTex;
                    gl_Position = P * V * fragPos;

    vec3 T = normalize(mat3(M) * vec3(1,0,0));
    vec3 B = normalize(mat3(M) * vec3(0,1,0));
    vec3 _N = normalize(mat3(M) * vec3(0,0,1));
    TBN = transpose(mat3(T, B, _N));
    TanLightPos = TBN * lightPos;
    TanViewPos = TBN * camPos;
    TanFragPos = TBN * fragPos.xyz;
                })",
                R"(#version 330 core
                in vec4 fragPos;
                in vec3 fragNor;
                in vec2 fragTex;

                in vec3 TanLightPos;
                in vec3 TanViewPos;
                in vec3 TanFragPos; 
                in mat3 TBN;
                uniform float height_scale;
                uniform sampler2D normalMap;
                uniform sampler2D displaceMap;
                uniform int layers;
                uniform float opacity;
                uniform bool showNormals;

                uniform sampler2D diffuseMap;
                uniform bool useTexture;
                uniform float ambient;
                uniform vec3 diffuseColor;
                uniform vec3 specularColor;
                uniform float shine;
                uniform vec3 camPos;
                uniform vec3 lightPos;
                uniform vec3 lightCol;
                uniform vec3 lightAtt;
                out vec4 color;
                void main() {
                    vec4 albedo = vec4(diffuseColor, 1.f);

                    vec3 viewDir = normalize(TanViewPos - TanFragPos);
                    float layerDepth = 1.0 / layers;
                    vec2 P = viewDir.xy * height_scale;
                    vec2 deltaTexCoords = P / layers;
                    float currentLayerDepth = 0.0;
                    vec2 currentTexCoords = fragTex;
                    float currentDepth = texture(displaceMap, fragTex).r;
                    while(currentLayerDepth < currentDepth) {
                        currentTexCoords -= deltaTexCoords;
                        currentDepth = texture(displaceMap, currentTexCoords).r;
                        currentLayerDepth += layerDepth;
                    }
                    vec2 texCoords = currentTexCoords;
                    
                    if (useTexture) {
                        albedo = texture(diffuseMap, texCoords);
                        if (albedo.a < 0.1f) {
                            discard;
                        }
                        albedo = vec4(0.325, 0.5, 0.1, 1.0);
                        albedo = albedo + (albedo * (texture(displaceMap, fragTex).r * 2.f - 1.f) * opacity);
                    }
                    vec3 N = normalize(TBN * normalize(texture(normalMap, texCoords).rgb) * 2.f - 1.f);
                    if (showNormals) { albedo = vec4((N + 1.0) / 2.0, 1.0); }
                    vec3 V = normalize(camPos - fragPos.xyz);
                    vec3 lightDir = lightPos - fragPos.xyz;
                    float lightDistance = length(lightDir);
                    vec3 L = normalize(lightDir);
                    float attFactor = 1;
                    if (length(lightAtt) > 0) {
                        attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
                    }
                    float lambert = dot(L, N);
                    vec3 H = normalize(L + V);
                    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f) / attFactor;
                    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine) / attFactor;
                    color.rgb = albedo.rgb * ambient +
                                albedo.rgb * diffuseContrib +
                                specularColor * specularContrib;
                    color.a = albedo.a;
                    })")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load light */
            if (auto light = Engine::getSingleComponent<LightComponent>()) {
                loadUniform("lightPos", light->getGameObject().getSpatial()->getPosition());
                loadUniform("lightCol", light->mColor);
                loadUniform("lightAtt", light->mAttenuation);
            }

            for (auto& renderable : Engine::getComponents<renderable::PhongRenderable>()) {
                auto meshComponent = renderable->getGameObject().getComponentByType<MeshComponent>();
                if (!meshComponent) {
                    continue;
                }

                /* Bind mesh */
                const Mesh & mesh(meshComponent->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                loadUniform("M", renderable->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("N", renderable->getGameObject().getSpatial()->getNormalMatrix());
                loadUniform("height_scale", hscale);
                loadUniform("layers", layers);
                loadUniform("opacity", opacity);
                loadUniform("showNormals", showNormals);

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
                if (auto normalMap = renderable->getGameObject().getComponentByType<NormalMapComponent>()) {
                    auto texture = (const Texture2D *)(normalMap->mTexture);
                    texture->bind();
                    loadUniform("normalMap", texture->mTextureID);
                }
                if (auto displaceMap = renderable->getGameObject().getComponentByType<DisplacementMapComponent>()) {
                    auto texture = (const Texture2D *)(displaceMap->mTexture);
                    texture->bind();
                    loadUniform("displaceMap", texture->mTextureID);
                }
 
 
                /* Bind material */
                if (auto matComp = renderable->getGameObject().getComponentByType<MaterialComponent>()) {
                    loadUniform("ambient", matComp->mAmbient);
                    loadUniform("diffuseColor", matComp->mDiffuse);
                    loadUniform("specularColor", matComp->mSpecular);
                    loadUniform("shine", matComp->mShine);
                }

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
    };
}
