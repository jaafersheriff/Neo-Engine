/* Cloud Shader class derives Shader
 * Contains reference to list of cloud billboards and entire rendering strategy */
#pragma once
#ifndef _CLOUD_SHADER_HPP_
#define _CLOUD_SHADER_HPP_

#include "Renderer/Shader.hpp"
#include "Cloud/CloudBillboard.hpp"
#include "Model/Texture.hpp"
#include "Light/Light.hpp"

class CloudShader : public Shader {
    public:
        /* Define GLSL shader locations */
        CloudShader() : Shader("../src/Cloud/CloudShader/cloud_vertex_shader.glsl",
                               "../src/Cloud/CloudShader/cloud_fragment_shader.glsl") { }

        /* Reference to render tartget */
        std::vector<CloudBillboard *> *billboards;

        /* Init render target and local shaders */
        bool init(std::vector<CloudBillboard *> *);

        /* Render */
        void render();

        /* Wrap up and shut down */
        void cleanUp();

    private:
        /* Utility functions */
        void sortByDistance();    /* Sort billboards by distance to camera */

        /* GLSL Load functions */
        void addAllLocations();
        void loadM(const glm::mat4 *);
        void loadCenter(const glm::vec3);
        void loadSize(const glm::vec2);
        void loadTexture(const Texture *);
};

#endif
