#version 450

in vec2 tex_coord;

uniform sampler2D tex_sampler_left;
uniform sampler2D tex_sampler_right;

layout(location = 0) out vec4 out_color;

void main()
{
  out_color = vec4(mix(texture(tex_sampler_left, tex_coord).rgb, texture(tex_sampler_right, tex_coord).rgb, 0.5), 1.f);
}
