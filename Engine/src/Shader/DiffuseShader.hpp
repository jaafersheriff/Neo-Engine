#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "Util/GlHelper.hpp"

namespace neo {

    class DiffuseShader : public Shader {

        public:

            DiffuseShader(const std::string &res) :
                Shader("Diffuse Shader",
                    _strdup("\
                        #version 330 core\n\
                        layout(location = 0) in vec3 vertPos;\
                        layout(location = 1) in vec3 vertNor;\
                        uniform mat4 P, V, M;\
                        uniform mat3 N;\
                        out vec4 fragPos;\
                        out vec3 fragNor;\
                        void main() {\
                            fragPos = M * vec4(vertPos, 1.0);\
                            fragNor = N * vertNor;\
                            gl_Position = P * V * fragPos;\
                        }"),
                    _strdup("\
                        #version 330 core\n\
                        in vec4 fragPos;\
                        in vec3 fragNor;\
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
                            color.rgb = diffuseColor * ambient +\
                                        diffuseColor * diffuseContrib +\
                                        specularColor * specularContrib;\
                            color.a = 1;\
                        }")
                )
            {}

            virtual void render(float dt, const RenderSystem &renderSystem) override {
                bind();

                /* Load PV */
                auto cameras = NeoEngine::getComponents<CameraComponent>();
                if (cameras.size()) {
                    loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                    loadMatrix(getUniform("V"), cameras.at(0)->getView());
                    loadVector(getUniform("camPos"), cameras.at(0)->getGameObject().getSpatial()->getPosition());
                }

                /* Load light */
                auto lights = NeoEngine::getComponents<LightComponent>();
                if (lights.size()) {
                    loadVector(getUniform("lightPos"), lights.at(0)->getGameObject().getSpatial()->getPosition());
                    loadVector(getUniform("lightCol"), lights.at(0)->getColor());
                    loadVector(getUniform("lightAtt"), lights.at(0)->getAttenuation());
                }

                for (auto diffuse : renderSystem.getRenderables<DiffuseShader, RenderableComponent>()) {
                    loadMatrix(getUniform("M"), diffuse->getGameObject().getSpatial()->getModelMatrix());
                    loadMatrix(getUniform("N"), diffuse->getGameObject().getSpatial()->getNormalMatrix());

                    /* Bind mesh */
                    const Mesh & mesh(diffuse->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.vaoId));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                    /* Bind material */
                    const Material & material(diffuse->getMaterial());
                    loadFloat(getUniform("ambient"), material.ambient);
                    loadVector(getUniform("diffuseColor"), material.diffuse);
                    loadVector(getUniform("specularColor"), material.specular);
                    loadFloat(getUniform("shine"), material.shine);

                    /* DRAW */
                    CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
                }

                CHECK_GL(glBindVertexArray(0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                unbind();
            }
    };
}
