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
#include <glm/glm.hpp>
#include <png++/png.hpp>

#include "visual.h"
#include "visual.vs.h"
#include "visual.fs.h"

using std::max;

class VisualImpl {

  public:
  void rotate();
  void read_texture();
  double time;

  GLuint program, vbo, vao, texture, vao_tex, vbo_tex;
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
  glDeleteTextures(1, &texture);
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

void build_spheres_mesh(float* p, float* pt, float c) {

  float s = sqrt(2.0)*c/(2.0+sqrt(2.0));

  std::vector<glm::vec3> all_tris, squares, slanted_squares, pyramids;
  std::vector<glm::vec2> all_tris_tex;

  // Build squares

  glm::vec3 x_axis(1.0, 0.0, 0.0);
  glm::vec3 y_axis(0.0, 1.0, 0.0);
  glm::vec3 z_axis(0.0, 0.0, 1.0);

  float senses[2] = {-1.0f, 1.0f };

  glm::vec3 vertex_sq[24];
  glm::vec2 vertex_sq_tex[24];

  for(int s0=0;s0<2;s0++) {
    for(int s1=0;s1<2;s1++) {
      for(int s2=0;s2<2;s2++) {
        vertex_sq[0*8+s0*4+s1*2+s2] = c*senses[s0]*x_axis
                                    + s*senses[s1]*y_axis
                                    + s*senses[s2]*z_axis;
        vertex_sq[1*8+s0*4+s1*2+s2] = s*senses[s0]*x_axis
                                    + c*senses[s1]*y_axis
                                    + s*senses[s2]*z_axis;
        vertex_sq[2*8+s0*4+s1*2+s2] = s*senses[s0]*x_axis
                                    + s*senses[s1]*y_axis
                                    + c*senses[s2]*z_axis;


        vertex_sq_tex[0*8+s0*4+s1*2+s2] = glm::vec2(s*0.5*(1+senses[s1])/c
                                                   ,s*0.5*(1+senses[s2])/c);
        vertex_sq_tex[1*8+s0*4+s1*2+s2] = glm::vec2(s*0.5*(1+senses[s0])/c
                                                   ,s*0.5*(1+senses[s2])/c);
        vertex_sq_tex[2*8+s0*4+s1*2+s2] = glm::vec2(s*0.5*(1+senses[s0])/c
                                                   ,s*0.5*(1+senses[s1])/c);
      }
    }
  }

  glm::vec3 vertex_pyr[8];

  float pyr_d = 0.5*(s+c);

  for(int s0=0;s0<2;s0++) {
    for(int s1=0;s1<2;s1++) {
      for(int s2=0;s2<2;s2++) {
        vertex_pyr[4*s0+2*s1+s2] = pyr_d*glm::vec3(senses[s0],senses[s1],senses[s2]);
      }
    }
  }

  std::vector<glm::vec3> square_faces;
  std::vector<glm::vec2> square_faces_tex;

  for(int a=0;a<3;a++) {
    for(int s0=0;s0<2;s0++) {
      // TODO - adjust winding
      int ao[12] =   { 0, 0
                     , 0, 1
                     , 1, 0
                     , 0, 1
                     , 1, 1
                     , 1, 0
                     };
      int ao1 = (a+1)%3;
      int ao2 = (a+2)%3;
      for(int i=0;i<6;i++) {
        int c[3];
        c[a] = s0;
        c[ao1] = ao[i*2+0];
        c[ao2] = ao[i*2+1];
        square_faces.push_back(vertex_sq[a*8+c[0]*4+c[1]*2+c[2]*1]);
        square_faces_tex.push_back(vertex_sq_tex[a*8+c[0]*4+c[1]*2+c[2]*1]);
      }
    }
  }

  std::vector<glm::vec3> hexagons;
  std::vector<glm::vec2> hexagons_tex;

  for(int a0=0;a0<3;a0++) {
    for(int a1=0;a1<a0;a1++) {
      for(int s0=0;s0<2;s0++) {
        for(int s1=0;s1<2;s1++) {

          int a2;
          if (a0 == 1 && a1 == 0) a2 = 2;
          if (a0 == 2 && a1 == 0) a2 = 1;
          if (a0 == 2 && a1 == 1) a2 = 0;
          int coords0[3], coords1[3];
          coords0[a0] = s0;
          coords0[a1] = s1;
          coords0[a2] = 1;
          hexagons.push_back(vertex_pyr[4*coords0[0]+2*coords0[1]+1*coords0[2]]);
          hexagons.push_back(vertex_sq[a1*8+coords0[0]*4+coords0[1]*2+coords0[2]*1]);
          hexagons.push_back(vertex_sq[a0*8+coords0[0]*4+coords0[1]*2+coords0[2]*1]);

          hexagons_tex.push_back(glm::vec2(0.5-0.5*(-c+s)/c,0.5));
          hexagons_tex.push_back(glm::vec2(0.5-s/c, 0.5+s/c));
          hexagons_tex.push_back(glm::vec2(0.5-s/c, 0.5-s/c));

          coords1[a0] = s0;
          coords1[a1] = s1;
          coords1[a2] = 0;
          hexagons.push_back(vertex_pyr[4*coords1[0]+2*coords1[1]+1*coords1[2]]);
          hexagons.push_back(vertex_sq[a1*8+coords1[0]*4+coords1[1]*2+coords1[2]*1]);
          hexagons.push_back(vertex_sq[a0*8+coords1[0]*4+coords1[1]*2+coords1[2]*1]);

          hexagons_tex.push_back(glm::vec2(0.5+0.5*(-c+s)/c,0.5));
          hexagons_tex.push_back(glm::vec2(0.5+s/c, 0.5+s/c));
          hexagons_tex.push_back(glm::vec2(0.5+s/c, 0.5-s/c));

          hexagons.push_back(vertex_sq[a1*8+coords0[0]*4+coords0[1]*2+coords0[2]*1]);
          hexagons.push_back(vertex_sq[a0*8+coords0[0]*4+coords0[1]*2+coords0[2]*1]);
          hexagons.push_back(vertex_sq[a0*8+coords1[0]*4+coords1[1]*2+coords1[2]*1]);

          hexagons_tex.push_back(glm::vec2(0.5-s/c, 0.5+s/c));
          hexagons_tex.push_back(glm::vec2(0.5-s/c, 0.5-s/c));
          hexagons_tex.push_back(glm::vec2(0.5+s/c, 0.5-s/c));

          hexagons.push_back(vertex_sq[a1*8+coords1[0]*4+coords1[1]*2+coords1[2]*1]);
          hexagons.push_back(vertex_sq[a1*8+coords0[0]*4+coords0[1]*2+coords0[2]*1]);
          hexagons.push_back(vertex_sq[a0*8+coords1[0]*4+coords1[1]*2+coords1[2]*1]);

          hexagons_tex.push_back(glm::vec2(0.5+s/c, 0.5+s/c));
          hexagons_tex.push_back(glm::vec2(0.5-s/c, 0.5+s/c));
          hexagons_tex.push_back(glm::vec2(0.5+s/c, 0.5-s/c));
        }
      }
    }
  }


  all_tris.insert(all_tris.end(), square_faces.begin(), square_faces.end());
  all_tris.insert(all_tris.end(), hexagons.begin(), hexagons.end());

  all_tris_tex.insert(all_tris_tex.end(), square_faces_tex.begin(), square_faces_tex.end());
  all_tris_tex.insert(all_tris_tex.end(), hexagons_tex.begin(), hexagons_tex.end());

  for(int i=0;i<all_tris.size();i++) {
    p[i*3+0] = all_tris[i][0];
    p[i*3+1] = all_tris[i][1];
    p[i*3+2] = all_tris[i][2];
  }

  for(int i=0;i<all_tris_tex.size();i++) {
    pt[i*2+0] = all_tris_tex[i][0];
    pt[i*2+1] = all_tris_tex[i][1];
  }
}

void Visual::initialize() {

  pimpl->program = compile_shader_program(
    visual_shader_vs_src,
    visual_shader_fs_src
  );
  std::cout << "Successfully compiled and linked shaders!" << std::endl;

  // Set up 2d drawing shape

  float c = 0.5, s = 0.1;

  static GLfloat vertex_pos_data[6*2*3*3+12*4*3*3];
  static GLfloat tex_pos_data[6*2*3*3+12*4*3*3];

  build_spheres_mesh(vertex_pos_data, tex_pos_data, c);
//    = {
//       // Front back
//       -s,  s, -c,
//       -s, -s, -c,
//        s, -s, -c,
//        s, -s, -c,
//       -s,  s, -c,
//        s,  s, -c,
//
//       -s,  s,  c,
//        s, -s,  c,
//       -s, -s,  c,
//        s, -s,  c,
//        s,  s,  c,
//       -s,  s,  c,
//
//       -c, -s,  s,
//       -c, -s, -s,
//       -c,  s, -s,
//       -c,  s, -s,
//       -c, -s,  s,
//       -c,  s,  s,
//
//        c, -s,  s,
//        c,  s, -s,
//        c, -s, -s,
//        c,  s, -s,
//        c,  s,  s,
//        c, -s,  s,
//
//        s, -c, -s,
//       -s, -c, -s,
//       -s, -c,  s,
//       -s, -c,  s,
//        s, -c, -s,
//        s, -c,  s,
//
//        s, c, -s,
//       -s, c,  s,
//       -s, c, -s,
//       -s, c,  s,
//        s, c,  s,
//        s, c, -s,
//
//        s
//  };

  glGenVertexArrays(1, &pimpl->vao);
  glBindVertexArray(pimpl->vao);
  glGenBuffers(1, &pimpl->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(6*2*3*3+12*4*3*3), vertex_pos_data, GL_STATIC_DRAW);

  glGenVertexArrays(1, &pimpl->vao_tex);
  glBindVertexArray(pimpl->vao_tex);
  glGenBuffers(1, &pimpl->vbo_tex);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo_tex);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(6*2*3*2+12*4*3*2), tex_pos_data, GL_STATIC_DRAW);

  png::image<png::rgb_pixel> image("src/pcb.png");
  png::image<png::rgb_pixel>::pixbuf pixbuf = image.get_pixbuf();
  int img_width  = (int)image.get_width();
  int img_height = (int)image.get_height();

  int imageSize = img_width * img_height * 3;
  unsigned char* data = new unsigned char[imageSize];

  for (int i = 0; i < imageSize; i += 3)
  {
    int x = (i / 3) % img_width;
    int y = (i / 3) / img_width;
    png::rgb_pixel pix = pixbuf.get_pixel((std::size_t)x, (std::size_t)y);

    data[i + 0] = pix.blue;
    data[i + 1] = pix.green;
    data[i + 2] = pix.red;
  }

  glGenTextures(1, &pimpl->texture);
  glBindTexture(GL_TEXTURE_2D, pimpl->texture);

  //delete data;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*) data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

void Visual::configure(double w, double h) {

  pimpl->width = w; pimpl->height = h;

}

void Visual::draw() {

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(pimpl->program);

  float mv[16] = {
    1, 0, 0, 0,
    0, 0.866, -0.5, 0,
    0, 0.5, 0.866, 0,
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

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo_tex);
  glVertexAttribPointer(
    1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    2,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pimpl->texture);
  glUniform1i(glGetUniformLocation(pimpl->program, "model_texture"), 0);

  glDrawArrays(GL_TRIANGLES, 0, 6*2*3+12*4*3);

}

void VisualImpl::rotate() {
  float theta = time*M_PI/2.0;
  proj[0] = cos(theta);
  proj[2] = -sin(theta);
  proj[8] = sin(theta);
  proj[10] = cos(theta);
}

void Visual::time_advance(double t) { pimpl->time += t; pimpl->rotate(); }

void Visual::time(double t) { pimpl->time = t; pimpl->rotate(); }

VisualImpl::VisualImpl() :
  time(0)
  {
}

