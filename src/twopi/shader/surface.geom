#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;

layout (location = 0) in vec3 in_position[];
layout (location = 1) in vec3 in_normal[];

layout (line_strip, max_vertices = 6) out;

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
  for (int i = 0; i < 3; i++)
  {
    vec3 p = in_position[i];

    gl_Position = camera.projection * camera.view * vec4(p, 1.f);
    frag_position = in_position[i];
    frag_normal = in_normal[i];
    EmitVertex();
    
    vec3 n = normalize(in_normal[i]);
    const float len = 0.1f;
    gl_Position = camera.projection * camera.view * vec4(p + n * len, 1.f);
    frag_position = in_position[i] + n * len;
    EmitVertex();

    EndPrimitive();
  }
}
