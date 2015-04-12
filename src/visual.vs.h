const char visual_shader_vs_src[] = "\
#version 330\n\
\n\
in vec3 pos;\n\
in vec2 pos_t;\n\
uniform mat4 proj;\n\
uniform mat4 mv;\n\
out vec4 posv;\n\
out vec2 pos_tv;\n\
\n\
void main() {\n\
  gl_Position = proj*mv*vec4(pos[0], pos[1], pos[2], 1);\n\
  posv = gl_Position;\n\
  pos_tv = pos_t;\n\
}\n\
\n\
\n\
";
