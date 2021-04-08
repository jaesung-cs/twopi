#ifndef TWOPI_VKE_VKE_SHADER_INFO_H_
#define TWOPI_VKE_VKE_SHADER_INFO_H_

#include <vector>

namespace twopi
{
namespace vke
{
struct Attribute;
struct Uniform;

class ShaderInfo
{
public:
  ShaderInfo();

  ~ShaderInfo();

private:
  std::vector<Attribute> attributes_;
  std::vector<Uniform> uniforms_;
};
}
}

#endif // TWOPI_VKE_VKE_SHADER_INFO_H_
