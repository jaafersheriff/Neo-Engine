#include "Display.hpp"
#include <iostream>

static void error_callback(int error, const char *desc) {
   std:: cerr << "Error: " << desc << std::endl;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
   glViewport(0, 0, width, height);
}

int Display::init() {
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
      return 2;
   }

   // Request version 3.2 of OpenGl
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

   window = glfwCreateWindow(this->width, this->height, "Neo", NULL, NULL);
   if (!window) {
      std::cerr << "Failed to create window" << std::endl;
      glfwTerminate();
      return 2;
   }
   glfwMakeContextCurrent(window);

   // GLEW
   glewExperimental = GL_FALSE;
   GLenum error = glGetError();
   if (error != GL_NO_ERROR) {
      std::cout << "OpenGL Error: " << error << std::endl;
      return 2;
   }
   error = glewInit();
   if (error != GLEW_OK) {
      std::cerr << "Failed to init GLEW" << std::endl;
      return 2;
   }
   glGetError();

   // Vsync
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1);

   // Resize callback
   glfwSetFramebufferSizeCallback(window, resize_callback);

   return 0;
}

void Display::setTitle(const char *name) {
   glfwSetWindowTitle(window, name);
}

void Display::update() { 
   glfwGetFramebufferSize(window, &width, &height);
   glViewport(0, 0, width, height);

   // Update fps
   double currTime = glfwGetTime();
   nbFrames++;
   if (currTime - lastTime >= 1.0) {
      std::cout << "FPS: " << double(nbFrames) << std::endl;
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
