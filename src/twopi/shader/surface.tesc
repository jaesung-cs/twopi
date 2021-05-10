#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 in_position[];
layout (location = 1) in vec3 in_normal[];

layout (vertices = 4) out;

layout (location = 0) out vec3 out_position[4];
layout (location = 1) out vec3 out_normal[4];

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = 32.f;
		gl_TessLevelInner[1] = 32.f;
		gl_TessLevelOuter[0] = 32.f;
		gl_TessLevelOuter[1] = 32.f;
		gl_TessLevelOuter[2] = 32.f;
		gl_TessLevelOuter[3] = 32.f;
  }

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	out_position[gl_InvocationID] = in_position[gl_InvocationID];
	out_normal[gl_InvocationID] = in_normal[gl_InvocationID];
}
