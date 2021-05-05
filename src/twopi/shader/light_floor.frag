#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 frag_position;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec2 frag_tex_coord;

layout (set = 0, binding = 0) uniform Camera
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

struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

const int MAX_NUM_LIGHTS = 8;
layout (std140, binding = 2) uniform LightUbo
{
  Light directional_lights[MAX_NUM_LIGHTS];
  Light point_lights[MAX_NUM_LIGHTS];
} lights;

// TODO
layout (constant_id = 0) const uint num_directional_lights = 1U;
layout (constant_id = 1) const uint num_point_lights = 1U;

layout (std140, binding = 3) uniform MaterialUbo
{
  vec3 specular;
  float shininess;
} material;

layout (location = 0) out vec4 out_color;

#include "core/light.h"

const float thickness = 0.05f;
const float r2a = 400.f;
float floor_pattern_strength(vec2 coord)
{
  coord = fract(coord);
  coord = min(coord, vec2(1.f) - coord);
  float x = min(coord.x, coord.y);
  return 1.f - smoothstep(0.f, thickness, x);
}

void main()
{
  // Directional light
  vec3 N = normalize(frag_normal);
  vec3 V = normalize(camera.eye - frag_position);
  
  float diffuse_strength = floor_pattern_strength(frag_tex_coord);
  
  // Axis color
  vec3 diffuse_color = vec3(0.f, 0.f, 0.f);
  if (abs(frag_tex_coord.y) <= thickness && min(fract(frag_tex_coord.x), 1.f - fract(frag_tex_coord.x)) >= abs(frag_tex_coord.y))
    diffuse_color.r = 1.f;
  if (abs(frag_tex_coord.x) <= thickness && min(fract(frag_tex_coord.y), 1.f - fract(frag_tex_coord.y)) >= abs(frag_tex_coord.x))
    diffuse_color.g = 1.f;
  if (frag_tex_coord.x + frag_tex_coord.y < 0.f)
    diffuse_color /= 5.f;
  diffuse_color = mix(vec3(1.f), diffuse_color, diffuse_strength);

  vec3 total_color = vec3(0.f, 0.f, 0.f);
  for (int i = 0; i < num_directional_lights; i++)
  {
    vec3 light_color = compute_directional_light_color(lights.directional_lights[i], diffuse_color, N, V);
    total_color += light_color;
  }
  
  for (int i = 0; i < num_point_lights; i++)
  {
    vec3 light_color = compute_point_light_color(lights.point_lights[i], diffuse_color, N, V);
    total_color += light_color;
  }

  const float r2 = 100.f;
  float d2 = dot(frag_tex_coord, frag_tex_coord);
  float alpha = 1.f;
  if (d2 >= r2 && d2 <= r2a)
    alpha *= 1.f - smoothstep(r2, r2a, d2);
  else if (d2 > r2a)
    alpha = 0.f;

  const float z_alpha_offset = 1.f;
  alpha *= smoothstep(0.f, z_alpha_offset, (inverse(model.model) * vec4(camera.eye, 1.f)).z) * 0.5f + 0.5f;

  // out_color = vec4(mix(texture(tex_sampler, frag_tex_coord).rgb, (frag_normal + 1.f) / 2.f, 0.5f), 1.f);
  out_color = vec4(total_color, alpha);
}
