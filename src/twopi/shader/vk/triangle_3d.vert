#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

layout (binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  mat4 model;
} camera;

layout (location = 0) out vec3 frag_color;

void main()
{
  gl_Position = camera.projection * camera.view * camera.model * vec4(position, 1.f);
  frag_color = color;
}
