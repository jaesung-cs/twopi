#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;

layout (std140, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  vec3 eye;
} camera;

layout (std140, binding = 1) uniform ModelUbo
{
  mat4 model;
  mat3 model_inverse_transpose;
} model;

void main()
{
  vec4 p = model.model * vec4(position, 1.f);
  gl_Position = camera.projection * camera.view * p;
}
