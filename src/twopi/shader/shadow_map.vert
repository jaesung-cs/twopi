#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex
layout (location = 0) in vec3 position;

layout (std140, binding = 1) uniform ModelUbo
{
  mat4 model;
  mat3 model_inverse_transpose;
} model;

layout (push_constant) uniform LightSourcePushConst
{
  mat4 projection;
	mat4 view;
  vec3 position;
} light_source;

layout (location = 0) out vec3 frag_position;

void main()
{
  vec4 p = model.model * vec4(position, 1.f);
  gl_Position = light_source.projection * light_source.view * p;
  frag_position = p.xyz / p.w;
}
