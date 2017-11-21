/* Abstract parent Renderer class 
 * Every feature will have its own derived Renderer */
#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "World/World.hpp"
#include "Shader/Shader.hpp"
#include "glm/glm.hpp"

class Renderer {
    public:
        /* Subrenderers need a pointer of a data structure to be rendered */
        // TODO templates

        /* Shaders are instantiated in the child renderer as the corresponding
         * render type */
        // TODO templates
        Shader *shader;
        
        /* Prepare handles everything that is needed to be done prioer to rendering
         * eg. organizing render data structure in a batch for optimized rendering */
        virtual void prepare() = 0;

        /* Provide renderer/shader with:
         * projection matrix
         * view matrix */
        virtual void setGlobals(const glm::mat4*, const glm::mat4*) = 0;

        /* Render provided data structure in the proper way  */
        // TODO : fix dyanmic cast shader. Use templates 
        virtual void render(const World *) = 0;

        /* Opposite of prepare()
         * Includes shader clean up */
        virtual void cleanUp() = 0;
};

#endif
