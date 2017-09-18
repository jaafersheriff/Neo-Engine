/////////////////////////////////////////////////////////////////////////////////////////
//                           Abstract parent Renderer class                            //
//                                                                                     //
// How to create a derived renderer:                                                   //
//    Renderer-specific parameters: data structure pointer to be renderered            //
//    acitvate(data structure to be rendered):                                         //
//       Set rendererer data structure pointer                                         //
//       Create corresponding derived shader                                           //
//       init shader                                                                   //
//    setGlobals(World):                                                               //
//       provide renderer with                                                         //
//          projection matrix                                                          //
//          view matrix                                                                //
//    render():                                                                        //
//       dynamic cast shader                                                           //
//       render data structure                                                         //
//       TODO                                                                          //
//    cleanUp():                                                                       //
//       shader cleanup                                                                //
/////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Shader/Shader.hpp"
#include "glm/glm.hpp"

class World;
class Renderer {
   public:
      Shader *shader;

      virtual void setGlobals(const glm::mat4*, const glm::mat4*) = 0;
      virtual void render(World *) = 0;
      virtual void cleanUp() = 0;
};

#endif
