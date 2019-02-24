#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"
#include "GLHelper/GlHelper.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    class PhongShadowedShader : public Shader {

        public:

            PhongShadowedShader(float b = 0.f) :
                Shader("PhongShadowed Shader", 
                    "#version 330 core\n\
                    layout(location = 0) in vec3 vertPos;\
                    layout(location = 1) in vec3 vertNor;\
                    layout(location = 2) in vec2 vertTex;\
                    uniform mat4 P, V, M, L;\
                    uniform mat3 N;\
                    out vec4 fragPos;\
                    out vec3 fragNor;\
                    out vec2 fragTex;\
                    out vec4 shadowCoord;\
                    void main() {\
                        fragPos = M * vec4(vertPos, 1.0);\
                        fragNor = N * vertNor;\
                        gl_Position = P * V * fragPos;\
                        fragTex = vertTex;\
                        shadowCoord = L * fragPos; \
                    }", 
                    "#version 330 core\n\
                    in vec4 fragPos;\
                    in vec3 fragNor;\
                    in vec2 fragTex;\
                    in vec4 shadowCoord;\
                    uniform float ambient;\
                    uniform vec3 diffuseColor;\
                    uniform vec3 specularColor;\
                    uniform float shine;\
                    uniform bool useTexture;\
                    uniform sampler2D diffuseMap;\
                    uniform vec3 camPos;\
                    uniform vec3 lightPos, lightCol, lightAtt;\
                    uniform bool useDotBias;\
                    uniform sampler2D shadowMap;\
                    uniform float bias;\
                    uniform int pcfSize;\
                    uniform bool usePCF;\
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
                        vec3 viewDir = camPos - fragPos.xyz;\
                        vec3 V = normalize(viewDir);\
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
                        float Tbias = bias;\
                        if (useDotBias) {\
                            Tbias = bias*tan(acos(clamp(lambert, 0, 1)));\
                            Tbias = clamp(Tbias, 0,0.01);\
                        }\
                        float visibility = 1.f;\
                        if (usePCF) {\
                            float shadow = 0.f;\
                            vec2 texelSize = 1.f / textureSize(shadowMap, 0);\
                            for (int x = -pcfSize; x <= pcfSize; x++) {\
                                for (int y = -pcfSize; y <= pcfSize; y++) {\
                                    float pcfDepth = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r;\
                                    shadow += shadowCoord.z - Tbias > pcfDepth ? 1.f : 0.f;\
                                }\
                            }\
                            shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);\
                            visibility = 1.f - shadow;\
                        }\
                        else if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - Tbias) {\
                            visibility = 0.f;\
                        }\
                        color.rgb = albedo.rgb * ambient + \
                                    visibility * albedo.rgb * diffuseContrib + \
                                    visibility * specularColor * specularContrib;\
                        color.a = albedo.a;\
                    }"),
                bias(b)
            {}

            bool useDotBias = true;
            float bias;
            const glm::mat4 biasMatrix = glm::mat4(
                0.5f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.5f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.5f, 0.5f, 0.5f, 1.0f);
            bool usePCF = true;
            int pcfSize = 2;

            virtual void render(const CameraComponent &camera) override {
                bind();

                /* Load Camera */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());
                loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    auto light = lights[0];
                    auto lightCam = light->getGameObject().getComponentByType<CameraComponent>();
                    loadUniform("lightPos", light->getGameObject().getSpatial()->getPosition());
                    loadUniform("lightCol", light->getColor());
                    loadUniform("lightAtt", light->getAttenuation());
                    loadUniform("L", biasMatrix * lightCam->getProj() * lightCam->getView());
                }

                /* Bias */
                loadUniform("bias", bias);
                loadUniform("useDotBias", useDotBias);

                /* PCF */
                loadUniform("usePCF", usePCF);
                loadUniform("pcfSize", pcfSize);

                /* Bind shadow map */
                const Texture & texture(*Loader::getFBO("depthMap")->mTextures[0]); 
                CHECK_GL(glActiveTexture(GL_TEXTURE0 + texture.mTextureID));
                CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture.mTextureID));
                loadUniform("shadowMap", texture.mTextureID);

                for (auto & model : MasterRenderer::getRenderables<PhongShadowedShader, RenderableComponent>()) {
                    loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                    loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

                    /* Bind mesh */
                    const Mesh & mesh(model->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                    /* Bind texture */
                    auto texComp = model->getGameObject().getComponentByType<DiffuseMapComponent>();
                    if (texComp) {
                        auto texture = (Texture2D &) (texComp->getTexture());
                        texture.bind();
                        loadUniform("diffuseMap", texture.mTextureID);
                        loadUniform("useTexture", true);
                    }
                    else {
                        loadUniform("useTexture", false);
                    }

                    /* Bind material */
                    auto materialComp = model->getGameObject().getComponentByType<MaterialComponent>();
                    if (materialComp) {
                        const Material &material = materialComp->getMaterial();
                        loadUniform("ambient", material.mAmbient);
                        loadUniform("diffuseColor", material.mDiffuse);
                        loadUniform("specularColor", material.mSpecular);
                        loadUniform("shine", material.mShine);
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
}
