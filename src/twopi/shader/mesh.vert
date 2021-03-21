#version 450

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;

void main()
{
  gl_Position = projection * view * model * vec4(position, 1.0);
  frag_color = color;
  frag_tex_coord = tex_coord;
}
