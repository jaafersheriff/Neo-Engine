#include "Display.hpp"
#include "Shader/GLSL.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream> /* cout, cerr */

/* GLFW error callback */
static void error_callback(int error, const char *desc) {
    std::cerr << "Error: " << desc << std::endl;
}

/* GLFW resize callback */
static void resize_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "Resizing: [" << width << ", " << height << "]" << std::endl;
}


int Display::init() {
    /* Set error callback */
    glfwSetErrorCallback(error_callback);

    /* Init GLFW */
    if(!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        return 1;
    }

    /* Request version 3.3 of OpenGL */
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    /* Create GLFW window */
    window = glfwCreateWindow(this->width, this->height, "Neo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    /* Init GLEW */
    glewExperimental = GL_FALSE;
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL Error: " << error << std::endl;
        return 1;
    }
    error = glewInit();
    if (error != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        return 1;
    }
    glGetError();

    /* Error check */
    GLSL::checkVersion();

    /* Vsync */
    glfwSwapInterval(1);

    /* Resize callback */
    glfwSetFramebufferSizeCallback(window, resize_callback);

    return 0;
}

void Display::setTitle(const char *name) {
    glfwSetWindowTitle(window, name);
}

void Display::update() { 
    /* Set viewport to window size */
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    /* Don't update display if window is minimized */
    if (!width && !height) {
        return;
    }
    
    /* Update projection matrix */
    projectionMatrix = glm::perspective(
              45.f,                 // fovy
              width/(float)height,  // aspect
              0.01f,                // near
              250.f);               // far
}

int Display::shouldClose() { 
    return glfwWindowShouldClose(window);
}

void Display::cleanUp() {
    /* Clean up GLFW */
    glfwDestroyWindow(window);
    glfwTerminate();
}
