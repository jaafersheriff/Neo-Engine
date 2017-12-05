/* Entity Shader class derives Shader
 * Maintains entity shader */
#pragma once
#ifndef _ENTITY_SHADER_HPP_
#define _ENTITY_SHADER_HPP_

#include "Shader/Shader.hpp"
#include "Light/Light.hpp"
#include "Model/Texture.hpp"

class EntityShader : public Shader {
    public:
        /* Define shader locations */
        EntityShader() : Shader("../src/Entity/EntityShader/entity_vertex_shader.glsl",
                                "../src/Entity/EntityShader/entity_fragment_shader.glsl") { }
        
        /* Init local shaders */
        bool init();

        /* Load functions */
        void loadP(const glm::mat4 *);
        void loadM(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadMaterial(const float, const glm::vec3, const glm::vec3);
        void loadShine(const float);
        void loadLight(const Light &);
        void loadUsesTexture(const bool);
        void loadTexture(const Texture *);
        void loadTime(const float);
};

#endif