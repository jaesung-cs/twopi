#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coord;

// Instance
layout (location = 3) in mat4 instance_transform;

layout (set = 0, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  mat4 model;
} camera;

layout (location = 0) out vec3 frag_color;
layout (location = 1) out vec2 frag_tex_coord;

void main()
{
  gl_Position = camera.projection * camera.view * camera.model * instance_transform * vec4(position, 1.f);
  frag_color = color;
  frag_tex_coord = tex_coord;
}
