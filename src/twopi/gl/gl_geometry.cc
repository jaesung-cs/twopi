#include <twopi/gl/gl_geometry.h>

#include <glad/glad.h>

namespace twopi
{
namespace gl
{
class Geometry::Impl
{
public:
  Impl()
  {
    glCreateVertexArrays(1, &vao_);
    glCreateBuffers(1, &ibo_);
  }

  ~Impl()
  {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &ibo_);
    glDeleteBuffers(vbos_.size(), vbos_.data());
  }

  template <typename T>
  void SetAttribute(int location, int size, const std::vector<T>& attribute)
  {
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    vbos_.push_back(vbo);
    glNamedBufferData(vbo, attribute.size() * sizeof(T), attribute.data(), GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(vao_, location);
    glVertexArrayVertexBuffer(vao_, location, vbo, 0, size * sizeof(T));
    glVertexArrayAttribFormat(vao_, location, size, GL_FLOAT, GL_FALSE, 0);
  }

  void SetElements(const std::vector<unsigned int>& elements)
  {
    count_ = elements.size();

    glNamedBufferData(ibo_, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
    glVertexArrayElementBuffer(vao_, ibo_);
  }

  void SetTriangles()
  {
    primitive_ = GL_TRIANGLES;
  }

  void SetLines()
  {
    primitive_ = GL_LINES;
  }

  void Draw()
  {
    glBindVertexArray(vao_);
    glDrawElements(primitive_, count_, GL_UNSIGNED_INT, 0);
  }

private:
  GLuint vao_ = 0;
  std::vector<GLuint> vbos_;
  GLuint ibo_ = 0;

  GLenum primitive_ = GL_TRIANGLES;

  int count_ = 0;
};

Geometry::Geometry()
{
  impl_ = std::make_unique<Impl>();
}

Geometry::~Geometry() = default;

template <typename T>
void Geometry::SetAttribute(int location, int size, const std::vector<T>& attribute)
{
  impl_->SetAttribute(location, size, attribute);
}

template void Geometry::SetAttribute(int location, int size, const std::vector<float>& attribute);
template void Geometry::SetAttribute(int location, int size, const std::vector<uint8_t>& attribute);

void Geometry::SetElements(const std::vector<unsigned int>& elements)
{
  impl_->SetElements(elements);
}

void Geometry::SetTriangles()
{
  impl_->SetTriangles();
}

void Geometry::SetLines()
{
  impl_->SetLines();
}

void Geometry::Draw()
{
  impl_->Draw();
}
}
}
