#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (quads, equal_spacing, ccw) in;

layout (location = 0) in vec3 in_position[];
layout (location = 1) in vec3 in_normal[];

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_normal;

layout (std140, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  vec3 eye;
} camera;

void main()
{
	// Interpolate UV coordinates
	vec3 pos1 = mix(in_position[0], in_position[1], gl_TessCoord.x);
	vec3 pos2 = mix(in_position[2], in_position[3], gl_TessCoord.x);
	frag_position = mix(pos1, pos2, gl_TessCoord.y);

	vec3 n1 = mix(in_normal[0], in_normal[1], gl_TessCoord.x);
	vec3 n2 = mix(in_normal[2], in_normal[3], gl_TessCoord.x);
	frag_normal = mix(n1, n2, gl_TessCoord.y);
  
  gl_Position = camera.projection * camera.view * vec4(frag_position, 1.f);
}
