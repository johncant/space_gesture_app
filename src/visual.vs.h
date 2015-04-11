const char visual_shader_vs_src[] = "\
#version 330\n\
\n\
in vec3 pos;\n\
uniform mat4 proj;\n\
uniform mat4 mv;\n\
out vec4 posv;\n\
\n\
void main() {\n\
  gl_Position = proj*mv*vec4(pos[0], pos[1], pos[2], 1);\n\
  posv = gl_Position;\n\
}\n\
\n\
\n\
";
