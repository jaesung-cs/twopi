#ifndef TWOPI_GL_GL_SHADER_H_
#define TWOPI_GL_GL_SHADER_H_

#include <memory>
#include <string>

#include <glm/fwd.hpp>

namespace twopi
{
namespace gl
{
namespace impl
{
class ShaderImpl;
}

class Shader
{
public:
  Shader() = delete;

  Shader(const std::string& vertex_shader_filepath, const std::string& fragment_shader_filepath);

  ~Shader();

  void Use();

  void UniformMatrix4f(const std::string& name, const glm::mat4& m);

private:
  std::unique_ptr<impl::ShaderImpl> impl_;
};
}
}

#endif // TWOPI_GL_GL_SHADER_H_
