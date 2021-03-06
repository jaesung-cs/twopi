#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (quads, equal_spacing, cw) in;

layout (location = 0) in vec3 in_position[];
layout (location = 1) in vec3 in_vx[];
layout (location = 2) in vec3 in_vy[];

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec3 frag_color;

layout (std140, binding = 0) uniform Camera
{
  mat4 projection;
  mat4 view;
  vec3 eye;
} camera;

void main()
{
  vec2 uv = gl_TessCoord.xy;

  vec3 p00 = in_position[0];
  vec3 p30 = in_position[1];
  vec3 p03 = in_position[2];
  vec3 p33 = in_position[3];
  
  vec3 p10 = p00 + in_vx[0] / 3.f;
  vec3 p20 = p30 - in_vx[1] / 3.f;
  vec3 p13 = p03 + in_vx[2] / 3.f;
  vec3 p23 = p33 - in_vx[3] / 3.f;
  
  vec3 p01 = p00 + in_vy[0] / 3.f;
  vec3 p02 = p03 - in_vy[2] / 3.f;
  vec3 p31 = p30 + in_vy[1] / 3.f;
  vec3 p32 = p33 - in_vy[3] / 3.f;

  vec3 p11 = p10 + p01 - in_position[0];
  vec3 p21 = p20 + p31 - in_position[1];
  vec3 p12 = p13 + p02 - in_position[2];
  vec3 p22 = p23 + p32 - in_position[3];

  float x0 = uv.x * uv.x * uv.x;
  float x1 = 3.f * uv.x * uv.x * (1.f - uv.x);
  float x2 = 3.f * uv.x * (1.f - uv.x) * (1.f - uv.x);
  float x3 = (1.f - uv.x) * (1.f - uv.x) * (1.f - uv.x);

  float y0 = uv.y * uv.y * uv.y;
  float y1 = 3.f * uv.y * uv.y * (1.f - uv.y);
  float y2 = 3.f * uv.y * (1.f - uv.y) * (1.f - uv.y);
  float y3 = (1.f - uv.y) * (1.f - uv.y) * (1.f - uv.y);

  frag_position = 
      p00 * x0 * y0 + p10 * x1 * y0 + p20 * x2 * y0 + p30 * x3 * y0
    + p01 * x0 * y1 + p11 * x1 * y1 + p21 * x2 * y1 + p31 * x3 * y1
    + p02 * x0 * y2 + p12 * x1 * y2 + p22 * x2 * y2 + p32 * x3 * y2
    + p03 * x0 * y3 + p13 * x1 * y3 + p23 * x2 * y3 + p33 * x3 * y3;
    
  float dx0 = 3.f * uv.x * uv.x;
  float dx1 = 6.f * uv.x * (1.f - uv.x);
  float dx2 = 3.f * (1.f - uv.x) * (1.f - uv.x);
  
  float dy0 = 3.f * uv.y * uv.y;
  float dy1 = 6.f * uv.y * (1.f - uv.y);
  float dy2 = 3.f * (1.f - uv.y) * (1.f - uv.y);

  vec3 vx = 
      (p10 - p00) * dx0 * y0 + (p20 - p10) * dx1 * y0 + (p30 - p20) * dx2 * y0
    + (p11 - p01) * dx0 * y1 + (p21 - p11) * dx1 * y1 + (p31 - p21) * dx2 * y1
    + (p12 - p02) * dx0 * y2 + (p22 - p12) * dx1 * y2 + (p32 - p22) * dx2 * y2
    + (p13 - p03) * dx0 * y3 + (p23 - p13) * dx1 * y3 + (p33 - p23) * dx2 * y3;

  vec3 vy = 
      (p01 - p00) * dy0 * x0 + (p02 - p01) * dy1 * x0 + (p03 - p02) * dy2 * x0
    + (p11 - p10) * dy0 * x1 + (p12 - p11) * dy1 * x1 + (p13 - p12) * dy2 * x1
    + (p21 - p20) * dy0 * x2 + (p22 - p21) * dy1 * x2 + (p23 - p22) * dy2 * x2
    + (p31 - p30) * dy0 * x3 + (p32 - p31) * dy1 * x3 + (p33 - p32) * dy2 * x3;

  frag_normal = normalize(cross(vx, vy));
  frag_color = (frag_normal + 1.f) / 2.f;
  gl_Position = camera.projection * camera.view * vec4(frag_position, 1.f);
}
