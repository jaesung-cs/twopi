#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 frag_position;
layout (location = 1) in vec3 frag_normal;

layout (set = 0, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  vec3 eye;
} camera;

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

void main()
{
  // Directional light
  vec3 N = normalize(frag_normal);
  vec3 V = normalize(camera.eye - frag_position);

  vec3 diffuse_color = (N + 1.f) / 2.f;

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

  out_color = vec4(total_color, 1.f);
}
