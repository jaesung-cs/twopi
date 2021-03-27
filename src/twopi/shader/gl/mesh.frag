#version 450

layout (location = 0) in vec3 frag_normal;
layout (location = 1) in vec2 frag_tex_coord;
layout (location = 2) in vec3 frag_position;

// Directional lights
// TODO: add point lights
struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Material
{
  vec3 ambient;
  bool has_diffuse_texture;
  sampler2D diffuse_sampler;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

const int MAX_NUM_LIGHTS = 8;
uniform int num_lights;
uniform Light lights[MAX_NUM_LIGHTS];
uniform Material material;
uniform vec3 eye;

layout (location = 0) out vec4 out_color;

vec3 compute_light_color(Light light, vec3 diffuse_color, vec3 N, vec3 V)
{
  vec3 L = normalize(light.position);
  vec3 R = reflect(-L, N);
  vec3 H = normalize(L + V);

  float diffuse_strength = max(dot(L, N), 0.f);
  float specular_strength = pow(max(dot(H, N), 0.f), material.shininess);

  vec3 color = 
    light.ambient * diffuse_color
    + diffuse_strength * light.diffuse * diffuse_color
    + specular_strength * light.specular * material.specular;

  return color;
}

void main()
{
  // Directional light
  vec3 N = normalize(frag_normal);
  vec3 V = normalize(eye - frag_position);

  vec3 diffuse_color;
  if (material.has_diffuse_texture)
    diffuse_color = texture(material.diffuse_sampler, frag_tex_coord).rgb;
  else
    diffuse_color = material.diffuse;

  vec3 total_color = vec3(0.f, 0.f, 0.f);
  for (int i = 0; i < num_lights; i++)
  {
    vec3 light_color = compute_light_color(lights[i], diffuse_color, N, V);
    total_color += light_color;
  }

  out_color = vec4(total_color, 1.f);
}
