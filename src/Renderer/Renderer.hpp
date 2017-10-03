// Abstract parent Renderer class
#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Shader/Shader.hpp"
#include "glm/glm.hpp"

class World;
class Renderer {
   public:
      /* Subrenderers need a pointer of a data structure to be rendered
       * TODO : learn to use templates for this 
       */

      /* Shaders are instantiated in the child renderer as the corresponding
       * render type
       * TODO : more templates 
       */
      Shader *shader;
      
      /* Prepare handles everything that is needed to be done prioer to rendering
       * eg. organizing render data structure in a batch for optimized rendering 
       */
      virtual void prepare() = 0;

      /* Provide renderer/shader with:
       * projection matrix
       * view matrix 
       */
      virtual void setGlobals(const glm::mat4*, const glm::mat4*) = 0;

      /* Render provided data structure in the proper way 
       * TODO : fix dyanmic cast shader. Use templates 
       */
      virtual void render(World *) = 0;

      /* Opposite of prepare()
       * Includes shader clean up
       */
      virtual void cleanUp() = 0;
};

#endif
