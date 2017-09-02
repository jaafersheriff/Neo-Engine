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

      GLFWwindow *window;     

      int init();
      void setTitle(const char *);
      int shouldClose();
      void update();
      void cleanUp();

   private:
      int initGLFW();
      const int FPSCAP = 60;
      double lastTime;
      double nbFrames;
};

#endif