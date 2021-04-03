#include <twopi/scene/geometry.h>

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

  void AddAttribute(std::vector<float>&& attribute)
  {
    attributes_.emplace_back(std::move(attribute));
  }

  void AddAttribute(const std::vector<float>& attribute)
  {
    attributes_.push_back(attribute);
  }

  const std::vector<float>& Attribute(int index) const
  {
    return attributes_[index];
  }

private:
  std::vector<std::vector<float>> attributes_;
};

Geometry::Geometry()
{
  impl_ = std::make_unique<Impl>();
}

Geometry::~Geometry() = default;

void Geometry::AddAttribute(std::vector<float>&& attribute)
{
  impl_->AddAttribute(std::move(attribute));
}

void Geometry::AddAttribute(const std::vector<float>& attribute)
{
  impl_->AddAttribute(attribute);
}

const std::vector<float>& Geometry::Attribute(int index) const
{
  return impl_->Attribute(index);
}
}
}
