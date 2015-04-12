#define GLFW_INCLUDE_GL_3
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
//#define __gl_h_
#include <GLFW/glfw3.h>
#ifdef __linux__
//#include <GL/glew.h>
//#include <GL/glext.h>
//#include <GL/gl.h>
#else
#include <OpenGL/gl3.h>
#endif

#include <iostream>
#include <math.h>
#include <map>
#include "visualizer.h"

static void error_callback(int error, const char* desc);
static void resize_callback(GLFWwindow* window, int width, int height);

class VisualizerImpl {
  friend class Visualizer;
  GLFWwindow* window;

  friend void error_callback(int, const char*);
  friend void resize_callback(GLFWwindow*, int, int);
  static std::map<GLFWwindow*, Visualizer*> window_visualizer_map;

  // Proxy to get round privateness from C callbacks
  static void proxy_configure(Visualizer& v, double w, double h) { v.configure(w,h); }

};

std::map<GLFWwindow*, Visualizer*> VisualizerImpl::window_visualizer_map;

static void error_callback(int error, const char* desc) {
  std::cerr << desc << std::endl;
};

static void resize_callback(GLFWwindow* window, int width, int height) {
  Visualizer& v = *(VisualizerImpl::window_visualizer_map[window]);
  VisualizerImpl::proxy_configure(v, double(width), double(height));
}

void Visualizer::configure(double width, double height) {
  glViewport(0, 0, nearbyint(width), nearbyint(height));
  visual->configure(width, height);

  // The resize event doesn't make glfwWaitEvents return, so
  draw();
}

VisualizerImpl* Visualizer::mkimpl() {
  return new VisualizerImpl;
}

void Visualizer::mainloop() {
  initialize();
  // Never return
  while(!glfwWindowShouldClose(pimpl->window)) {

    glfwMakeContextCurrent(pimpl->window);
    double t = glfwGetTime();
    visual->time(t);
    std::cout << t << std::endl;
    draw();

//    glfwWaitEvents();
    glfwPollEvents();

  }
}

void Visualizer::initialize() {
  int argc = 0;
  char* argv0 = NULL;

  glfwSetErrorCallback(error_callback);
  glfwInit();
  //glewInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  pimpl->window = glfwCreateWindow(400, 400, "foo", NULL, NULL);
  VisualizerImpl::window_visualizer_map.insert(std::make_pair(pimpl->window, this));

  glfwSetWindowSizeCallback(pimpl->window, resize_callback);

  if (!pimpl->window) {
    glfwTerminate();
  }

  glfwMakeContextCurrent(pimpl->window);
  visual->initialize();

  configure(400, 400);

  glfwSetErrorCallback(NULL);
}

void Visualizer::draw() {
  glfwMakeContextCurrent(pimpl->window);

  visual->draw();

  glfwSwapBuffers(pimpl->window);
}

Visualizer::~Visualizer() {
  if (pimpl->window) {
    glfwDestroyWindow(pimpl->window);
    glfwTerminate();
  }
  delete pimpl;
  delete visual;
}

void Visualizer::destroy() {
  glfwDestroyWindow(pimpl->window);
  glfwTerminate();
}

