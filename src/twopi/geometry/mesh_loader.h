#ifndef TWOPI_GEOMETRY_MESH_LAODER_H_
#define TWOPI_GEOMETRY_MESH_LAODER_H_

#include <memory>
#include <string>

namespace twopi
{
namespace geometry
{
class Mesh;

class MeshLoader
{
public:
  MeshLoader();
  ~MeshLoader();

  std::shared_ptr<Mesh> Load(const std::string& filepath);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GEOMETRY_MESH_LAODER_H_
