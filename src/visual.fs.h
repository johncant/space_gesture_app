const char visual_shader_fs_src[] = "\n\
#version 330\n\
\n\
out vec4 wtfFragColor;\n\
\n\
in vec4 posv;\n\
void main() {\n\
  wtfFragColor = vec4(1.0, 0.0, 0.0, 1.0);\n\
}\n\
\n\
";
