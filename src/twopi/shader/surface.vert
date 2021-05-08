#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (std140, binding = 1) uniform ModelUbo
{
  mat4 model;
  mat3 model_inverse_transpose;
} model;

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_normal;

void main()
{
  vec4 p = model.model * vec4(position, 1.f);

  // Projection is done in tess evaluation shader
  gl_Position = p;

  frag_position = p.xyz / p.w;
  frag_normal = model.model_inverse_transpose * normal;
}
