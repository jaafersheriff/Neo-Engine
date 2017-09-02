#include "Display.hpp"
#include <iostream>

static void error_callback(int error, const char *desc) {
   std:: cerr << "Error: " << desc << std::endl;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
   glViewport(0, 0, width, height);
}

int Display::init(const int width, const int height) {
   this->width = width;
   this->height = height;
   
   if (initGLFW()) {
      return 1;
   }

   lastTime = glfwGetTime();
   nbFrames = 0;

   std::cout << "Display initialized" << std::endl;

   return 0;
}

int Display::initGLFW() {
   glfwSetErrorCallback(error_callback);
   if(!glfwInit()) {
      std::cerr << "Error initializing GLFW" << std::endl;
      return 1;
   }

   // Request version 3.2 of OpenGl
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

   window = glfwCreateWindow(this->width, this->height, "Neo", NULL, NULL);
   if (!window) {
      std::cerr << "Failed to create window" << std::endl;
      glfwTerminate();
      return 1;
   }
   glfwMakeContextCurrent(window);

   // GLEW
   glewExperimental = GL_FALSE;
   if (glGetError() != GL_NO_ERROR) {
      std::cout << "OpenGL Error" << std::endl;
   }
   if (glewInit() != GLEW_OK) {
      std::cerr << "Failed to init GLEW" << std::endl;
      return 1;
   }
   glGetError();

   // Vsync
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1);

   // Resize callback
   glfwSetFramebufferSizeCallback(window, resize_callback);

   return 0;
}

void Display::update() { 
   glfwGetFramebufferSize(window, &width, &height);
   glViewport(0, 0, width, height);

   // Update fps
   double currTime = glfwGetTime();
   nbFrames++;
   if (currTime - lastTime >= 1.0) {
      std::cout << 1000.0 / double(nbFrames) << std::endl;
      nbFrames = 0;
      lastTime = currTime;
   }
}

int Display::shouldClose() { 
   return glfwWindowShouldClose(window);
}

void Display::cleanUp() {
   glfwDestroyWindow(window);
   glfwTerminate();
}
