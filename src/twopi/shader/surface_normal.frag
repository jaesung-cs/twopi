#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 frag_position; // unused
layout (location = 1) in vec3 frag_normal; // unused
layout (location = 2) in vec3 frag_color;

layout (location = 0) out vec4 out_color;

void main()
{
  out_color = vec4(frag_color, 1.f);
  // out_color = vec4(1.f, 0.f, 0.f, 1.f);
}
