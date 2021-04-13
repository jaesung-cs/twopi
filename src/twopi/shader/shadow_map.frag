#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 frag_position;

layout (push_constant) uniform LightSourcePushConst
{
  mat4 projection;
	mat4 view;
  vec3 position;
} light_source;

layout (location = 0) out float out_depth;

void main()
{
  out_depth = length(light_source.position - frag_position);
}
