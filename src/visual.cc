#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GL_3
#define GL_GLEXT_PROTOTYPES

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
#include <algorithm>
#include <stdio.h>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometry.hpp>

#include "visual.h"
#include "visual.vs.h"
#include "visual.fs.h"

using std::max;

class VisualImpl {

  public:
  double time;

  GLuint program, vbo, vao;
  unsigned int width, height;
  friend class Visual;

  VisualImpl();
  ~VisualImpl();

  float proj[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

};

VisualImpl::~VisualImpl() {
}

static GLuint compile_shader(GLenum shader_type, const char* source) {
  GLuint shader = glCreateShader(shader_type);

  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint info_log_length, compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

  if (!compile_status) {
    char error_log[info_log_length];
    glGetShaderInfoLog(shader, info_log_length, NULL, error_log);

    std::cout << error_log << std::endl;
  }

  return shader;
}

static GLuint compile_shader_program(
  const char* vertex_shader_source,
  const char* fragment_shader_source
 ) {
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

  GLuint program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint link_status, info_log_length;

  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

  if (!link_status) {

    char error_log[info_log_length];
    glGetProgramInfoLog(program, info_log_length, NULL, error_log);

    std::cout << error_log << std::endl;
  }

  return program;
}


Visual::Visual() :
  pimpl(new VisualImpl())
{
}

Visual::~Visual() {
  delete pimpl;
}

void Visual::initialize() {

  pimpl->program = compile_shader_program(
    visual_shader_vs_src,
    visual_shader_fs_src
  );
  std::cout << "Successfully compiled and linked shaders!" << std::endl;

  // Set up 2d drawing shape
  glGenVertexArrays(1, &pimpl->vao);
  glBindVertexArray(pimpl->vao);

  static const GLfloat test_data[] = {
       -1.0f,  1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
       -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f, 0.0f
  };

  glGenBuffers(1, &pimpl->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(test_data), test_data, GL_STATIC_DRAW);

//  glGenTextures(1, &pimpl->texture);
}

void Visual::configure(double w, double h) {

  pimpl->width = w; pimpl->height = h;

}

void Visual::draw() {

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(pimpl->program);

  float mv[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  glUniformMatrix4fv(glGetUniformLocation(pimpl->program, "proj"), 1, false, pimpl->proj);
  glUniformMatrix4fv(glGetUniformLocation(pimpl->program, "mv"), 1, false, mv);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo);
  glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

//  glActiveTexture(GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_1D_ARRAY, pimpl->texture);
  glUniform1i(glGetUniformLocation(pimpl->program, "sampler"), 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

}

void Visual::time_advance(double t) { pimpl->time += t; }

void Visual::time(double t) { pimpl->time = t; }

VisualImpl::VisualImpl() :
  time(0)
  {
}

