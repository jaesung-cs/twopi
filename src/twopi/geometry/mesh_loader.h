#ifndef TWOPI_GEOMETRY_MESH_LAODER_H_
#define TWOPI_GEOMETRY_MESH_LAODER_H_

#include <memory>
#include <string>

namespace twopi
{
namespace geometry
{
class Mesh;

namespace impl
{
class MeshLoaderImpl;
}

class MeshLoader
{
public:
  MeshLoader();
  ~MeshLoader();

  std::shared_ptr<Mesh> Load(const std::string& filepath);

private:
  std::unique_ptr<impl::MeshLoaderImpl> impl_;
};
}
}

#endif // TWOPI_GEOMETRY_MESH_LAODER_H_
