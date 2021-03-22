#include <twopi/physics/cloth/mesh_cloth.h>

#include <vector>

#include <Eigen/Dense>

namespace twopi
{
namespace physics
{
class MeshCloth::Impl
{
public:
  Impl() = delete;

  explicit Impl(int subdivision)
    : subdivision_(subdivision)
  {
    positions_.resize(subdivision + 1);

    // Start at height 1m
    constexpr auto z = 1.;

    for (int r = 0; r <= subdivision; r++)
    {
      const auto x = static_cast<double>(r) / subdivision * 2. - 1.;

      for (int c = 0; c <= subdivision; c++)
      {
        const auto y = static_cast<double>(c) / subdivision * 2. - 1.;

        positions_[r].emplace_back(Eigen::Vector3d{ x, y, z });
      }
    }
  }

  ~Impl() = default;

private:
  int subdivision_ = 1;

  std::vector<std::vector<Eigen::Vector3d>> positions_;
};

MeshCloth::MeshCloth(int subdivision)
{
  impl_ = std::make_unique<Impl>(subdivision);
}

MeshCloth::~MeshCloth() = default;
}
}
