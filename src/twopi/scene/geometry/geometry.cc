#include <twopi/scene/geometry/geometry.h>

namespace twopi
{
namespace scene
{
class Geometry::Impl
{
public:
  Impl()
  {
  }

  ~Impl()
  {
  }

private:
};

Geometry::Geometry()
{
  impl_ = std::make_unique<Impl>();
}

Geometry::~Geometry() = default;
}
}
