/* Entity Shader class derives Shader
 * Contains reference to a list of entities and entire rendering strategy */
#pragma once
#ifndef _ENTITY_SHADER_HPP_
#define _ENTITY_SHADER_HPP_

#include "Entity/Entity.hpp"
#include "Shader/Shader.hpp"
#include "Light/Light.hpp"
#include "Model/Texture.hpp"

#include <vector>

class EntityShader : public Shader {
    public:
        /* Define GLSL shader locations */
        EntityShader() : Shader("../src/Entity/EntityShader/entity_vertex_shader.glsl",
                                "../src/Entity/EntityShader/entity_fragment_shader.glsl") { }
        
        /* Reference to render target */
        std::vector<Entity *> *entitiesPointer;

        /* Init render target and local shaders */
        bool init(std::vector<Entity *> *);

        /* Render */
        void render(const World *);

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* Mesh */
        void loadMesh(const Mesh *);
        void unloadMesh(const Mesh *);
        
        /* Texture */
        void loadTexture(const ModelTexture &);
        void unloadTexture(const ModelTexture &);

        /* GLSL Load functions */
        void addAllLocations();
        void setGlobals(const glm::mat4 *, const glm::mat4 *);
        void loadP(const glm::mat4 *);
        void loadM(const glm::mat4 *);
        void loadV(const glm::mat4 *);
        void loadCameraPos(const glm::vec3);
        void loadMaterial(const float, const glm::vec3, const glm::vec3);
        void loadShine(const float);
        void loadLight(const Light *);
        void loadUsesTexture(const bool);
        void loadTexture(const Texture *);
};

#endif
