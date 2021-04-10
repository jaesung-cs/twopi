#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

// Instance
layout (location = 3) in mat4 instance_transform;

layout (set = 0, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  vec3 eye;
} camera;

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec2 frag_tex_coord;

void main()
{
  gl_Position = camera.projection * camera.view * instance_transform * vec4(position, 1.f);

  vec4 p = instance_transform * vec4(position, 1.f);
  frag_position = p.xyz / p.w;
  frag_normal = transpose(inverse(mat3(instance_transform))) * normal;
  frag_tex_coord = tex_coord;
}
