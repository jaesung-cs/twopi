#version 450

in vec2 tex_coord;

uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main()
{
  out_color = vec4(1.f - texture(tex_sampler, tex_coord).rgb, 1.f);
}
