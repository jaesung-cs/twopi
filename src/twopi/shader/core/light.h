vec3 compute_directional_light_color(Light light, vec3 diffuse_color, vec3 N, vec3 V)
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

vec3 compute_point_light_color(Light light, vec3 diffuse_color, vec3 N, vec3 V)
{
  float d = length(light.position - frag_position);
  float atten = 1.f / (0.1f * d * d + 1.f);
  vec3 L = normalize(light.position - frag_position);
  vec3 R = reflect(-L, N);
  vec3 H = normalize(L + V);

  float diffuse_strength = max(dot(L, N), 0.f);
  float specular_strength = pow(max(dot(H, N), 0.f), material.shininess);

  vec3 color = atten * (
    light.ambient * diffuse_color
    + diffuse_strength * light.diffuse * diffuse_color
    + specular_strength * light.specular * material.specular);

  return color;
}
