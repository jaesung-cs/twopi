#ifndef TWOPI_VKE_VKE_UNIFORM_H_
#define TWOPI_VKE_VKE_UNIFORM_H_

namespace twopi
{
namespace vke
{
struct Uniform
{
  enum class Type
  {
    BUFFER,
    SAMPLER,
  };

  Uniform() = delete;

  Uniform(int location, Type type)
    : location(location), type(type)
  {
  }

  int location;
  Type type;
};
}
}

#endif // TWOPI_VKE_VKE_UNIFORM_H_
