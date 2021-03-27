#version 450

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 model_inverse_transpose;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

layout (location = 0) out vec3 frag_normal;
layout (location = 1) out vec2 frag_tex_coord;
layout (location = 2) out vec3 frag_position;

void main()
{
  vec4 model_position = model * vec4(position, 1.0);
  frag_position = model_position.xyz / model_position.w;
  gl_Position = projection * view * model_position;
  frag_tex_coord = tex_coord;
  frag_normal = normalize(model_inverse_transpose * normal);
}
