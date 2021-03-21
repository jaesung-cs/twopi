#version 450

layout(location = 0) in vec2 position;

out vec2 tex_coord;

void main()
{
  gl_Position = vec4(2.f * (position - 0.5f), 0.f, 1.f);
  tex_coord = position;
}
