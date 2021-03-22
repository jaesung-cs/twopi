#ifndef TWOPI_GEOMETRY_MESH_H_
#define TWOPI_GEOMETRY_MESH_H_

#include <memory>
#include <vector>
#include <string>

namespace twopi
{
namespace geometry
{
class Mesh
{
public:
  Mesh();
  ~Mesh();

  void SetVertices(std::vector<float>&& vertices);
  void SetNormals(std::vector<float>&& normals);
  void SetTexCoords(std::vector<float>&& tex_coords);
  void SetIndices(std::vector<uint32_t>&& indices);
  void SetTextureFilepath(std::string&& texture_filepath);

  const std::vector<float>& Vertices() const;
  const std::vector<float>& Normals() const;
  const std::vector<float>& TexCoords() const;
  const std::vector<uint32_t>& Indices() const;
  const std::string& TextureFilepath() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GEOMETRY_MESH_H_
