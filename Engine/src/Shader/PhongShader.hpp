#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"
#include "GLHelper/GlHelper.hpp"

using namespace neo;

class PhongShader : public Shader {

public:

    PhongShader() :
        Shader("Phong Shader",
            "#version 330 core\n\
                layout(location = 0) in vec3 vertPos;\
                layout(location = 1) in vec3 vertNor;\
                layout(location = 2) in vec2 vertTex;\
                uniform mat4 P, V, M;\
                uniform mat3 N;\
                out vec4 fragPos;\
                out vec3 fragNor;\
                out vec2 fragTex;\
                void main() {\
                    fragPos = M * vec4(vertPos, 1.0);\
                    fragNor = N * vertNor;\
                    fragTex = vertTex;\
                    gl_Position = P * V * fragPos;\
                }",
            "#version 330 core\n\
                in vec4 fragPos;\
                in vec3 fragNor;\
                in vec2 fragTex;\
                uniform sampler2D diffuseMap;\
                uniform bool useTexture;\
                uniform float ambient;\
                uniform vec3 diffuseColor;\
                uniform vec3 specularColor;\
                uniform float shine;\
                uniform vec3 camPos;\
                uniform vec3 lightPos;\
                uniform vec3 lightCol;\
                uniform vec3 lightAtt;\
                out vec4 color;\
                void main() {\
                    vec4 albedo = vec4(diffuseColor, 1.f);\
                    if (useTexture) {\
                        albedo = texture(diffuseMap, fragTex);\
                        if (albedo.a < 0.1f) {\
                            discard;\
                        }\
                    }\
                    vec3 N = normalize(fragNor);\
                    vec3 V = normalize(camPos - fragPos.xyz);\
                    vec3 lightDir = lightPos - fragPos.xyz;\
                    float lightDistance = length(lightDir);\
                    vec3 L = normalize(lightDir);\
                    float attFactor = 1;\
                    if (length(lightAtt) > 0) {\
                        attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;\
                    }\
                    float lambert = dot(L, N);\
                    vec3 H = normalize(L + V);\
                    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f) / attFactor;\
                    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine) / attFactor;\
                    color.rgb = albedo.rgb * ambient +\
                                albedo.rgb * diffuseContrib +\
                                specularColor * specularContrib;\
                    color.a = albedo.a;\
                    }")
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load light */
            auto lights = NeoEngine::getComponents<LightComponent>();
            if (lights.size()) {
                loadUniform("lightPos", lights.at(0)->getGameObject().getSpatial()->getPosition());
                loadUniform("lightCol", lights.at(0)->mColor);
                loadUniform("lightAtt", lights.at(0)->mAttenuation);
            }

            for (auto & model : MasterRenderer::getRenderables<PhongShader, RenderableComponent>()) {
                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                /* Bind texture */
                if (auto diffuseMap = model->getGameObject().getComponentByType<DiffuseMapComponent>()) {
                    auto texture = (const Texture2D *) (diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("diffuseMap", texture->mTextureID);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* Bind material */
                if (auto matComp = model->getGameObject().getComponentByType<MaterialComponent>()) {
                    loadUniform("ambient", matComp->mAmbient);
                    loadUniform("diffuseColor", matComp->mDiffuse);
                    loadUniform("specularColor", matComp->mSpecular);
                    loadUniform("shine", matComp->mShine);
                }

                /* DRAW */
                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            CHECK_GL(glActiveTexture(GL_TEXTURE0));
            CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
            unbind();
        }
};
