#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 out_color;

void main()
{
  vec3 white = vec3(1.f, 1.f, 1.f);
  out_color = vec4(white, 1.f);
}
