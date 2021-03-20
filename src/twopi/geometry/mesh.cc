#include <twopi/geometry/mesh.h>

namespace twopi
{
namespace geometry
{
namespace impl
{
class MeshImpl
{
public:
  MeshImpl()
  {
  }

  ~MeshImpl() = default;

  void SetVertices(std::vector<float>&& vertices)
  {
    vertices_ = std::move(vertices);
  }

  void SetNormals(std::vector<float>&& normals)
  {
    normals_ = std::move(normals);
  }

  void SetTexCoords(std::vector<float>&& tex_coords)
  {
    tex_coords_ = std::move(tex_coords);
  }

  void SetIndices(std::vector<uint32_t>&& indices)
  {
    indices_ = std::move(indices);
  }

  void SetTextureFilepath(std::string&& texture_filepath)
  {
    texture_filepath_ = std::move(texture_filepath);
  }

  const std::vector<float>& Vertices() const
  {
    return vertices_;
  }

  const std::vector<float>& Normals() const
  {
    return normals_;
  }

  const std::vector<float>& TexCoords() const
  {
    return tex_coords_;
  }

  const std::vector<uint32_t>& Indices() const
  {
    return indices_;
  }

  const std::string& TextureFilepath() const
  {
    return texture_filepath_;
  }

private:
  std::vector<float> vertices_;
  std::vector<float> normals_;
  std::vector<float> tex_coords_;
  std::vector<uint32_t> indices_;
  std::string texture_filepath_;
};
}

Mesh::Mesh()
{
  impl_ = std::make_unique<impl::MeshImpl>();
}

Mesh::~Mesh() = default;

void Mesh::SetVertices(std::vector<float>&& vertices)
{
  impl_->SetVertices(std::move(vertices));
}

void Mesh::SetNormals(std::vector<float>&& normals)
{
  impl_->SetNormals(std::move(normals));
}

void Mesh::SetTexCoords(std::vector<float>&& tex_coords)
{
  impl_->SetTexCoords(std::move(tex_coords));
}

void Mesh::SetIndices(std::vector<uint32_t>&& indices)
{
  impl_->SetIndices(std::move(indices));
}

void Mesh::SetTextureFilepath(std::string&& texture_filepath)
{
  impl_->SetTextureFilepath(std::move(texture_filepath));
}

const std::vector<float>& Mesh::Vertices() const
{
  return impl_->Vertices();
}

const std::vector<float>& Mesh::Normals() const
{
  return impl_->Normals();
}

const std::vector<float>& Mesh::TexCoords() const
{
  return impl_->TexCoords();
}

const std::vector<uint32_t>& Mesh::Indices() const
{
  return impl_->Indices();
}

const std::string& Mesh::TextureFilepath() const
{
  return impl_->TextureFilepath();
}
}
}
