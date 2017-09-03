#pragma once
#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Display {
   public:
      int width = 1280;
      int height = 960;
      double fps = 0;

      GLFWwindow *window;     

      int init();
      void setTitle(const char *);
      int shouldClose();
      void update();
      void cleanUp();

   private:
      double lastTime;
      double nbFrames;
};

#endif