const char visual_shader_fs_src[] = "\n\
#version 330\n\
\n\
out vec4 wtfFragColor;\n\
uniform sampler2D model_texture;\n\
\n\
in vec4 posv;\n\
in vec2 pos_tv;\n\
void main() {\n\
  vec4 wtfFragColor1 = vec4(1.0f*pos_tv[0], 1.0f*pos_tv[1], 0, 1.0f);\n\
  wtfFragColor = texture2D(model_texture, pos_tv);\n\
  wtfFragColor[3] = 1.0f;\n\
  wtfFragColor = 1.0*wtfFragColor+0.0*wtfFragColor1;\n\
}\n\
\n\
";
