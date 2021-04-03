#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec2 frag_tex_coord;

layout (binding = 1) uniform sampler2D tex_sampler;

layout (location = 0) out vec4 out_color;

void main()
{
  // out_color = vec4(frag_color, 1.f);
  // out_color = vec4(frag_tex_coord, 0.f, 1.f);
  out_color = vec4(texture(tex_sampler, frag_tex_coord).rgb, 1.f);
}