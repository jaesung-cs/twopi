#include <twopi/vke/vke_model.h>

#include <twopi/vkw/vkw_buffer.h>
#include <twopi/vkw/vkw_device_memory.h>

namespace twopi
{
namespace vke
{
class Model::Impl
{
public:
  Impl()
  {
  }

  ~Impl()
  {
  }

private:
  // Buffer
};

Model::Model()
  : impl_(std::make_unique<Impl>())
{
}

Model::~Model() = default;
}
}
