#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

using namespace neo;

class PhongShader : public Shader {

    public: 
    
        PhongShader(RenderSystem &r) :
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
                    vec3 diffColor = diffuseColor;\
                    color.a = 1.f;\
                    if (useTexture) {\
                        vec4 texColor = texture(diffuseMap, fragTex);\
                        if (texColor.a < 0.1) {\
                            discard;\
                        }\
                        diffColor = texColor.rgb;\
                        color.a = texColor.a;\
                    }\
                    color.rgb = diffColor.rgb * ambient +\
                                diffColor.rgb * diffuseContrib +\
                                specularColor * specularContrib;\
                    }")
        {}

        virtual void render(const RenderSystem &renderSystem, const CameraComponent &camera) override {
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Load light */
            auto lights = NeoEngine::getComponents<LightComponent>();
            if (lights.size()) {
                loadUniform("lightPos", lights.at(0)->getGameObject().getSpatial()->getPosition());
                loadUniform("lightCol", lights.at(0)->getColor());
                loadUniform("lightAtt", lights.at(0)->getAttenuation());
            }

            for (auto model : renderSystem.getRenderables<PhongShader, RenderableComponent>()) {
                loadUniform("M", model->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("N", model->getGameObject().getSpatial()->getNormalMatrix());

                /* Bind mesh */
                const Mesh & mesh(model->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                /* Bind texture */
                auto texComp = model->getGameObject().getComponentByType<TextureComponent>();
                if (texComp) {
                    auto texture = (Texture2D &) (texComp->getTexture());
                    texture.bind();
                    loadUniform("diffuseMap", texture.textureId);
                    loadUniform("useTexture", true);
                }
                else {
                    loadUniform("useTexture", false);
                }

                /* Bind material */
                auto matComp = model->getGameObject().getComponentByType<MaterialComponent>();
                if (matComp) {
                    const Material & material(matComp->getMaterial());
                    loadUniform("ambient", material.ambient);
                    loadUniform("diffuseColor", material.diffuse);
                    loadUniform("specularColor", material.specular);
                    loadUniform("shine", material.shine);
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
