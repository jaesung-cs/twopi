#ifndef TWOPI_VKE_VKE_MODEL_H_
#define TWOPI_VKE_VKE_MODEL_H_

#include <memory>

namespace twopi
{
namespace vke
{
class Model
{
public:
  Model();
  ~Model();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_VKE_VKE_MODEL_H_
