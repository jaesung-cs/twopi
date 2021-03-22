#ifndef TWOPI_GL_GL_SHADER_H_
#define TWOPI_GL_GL_SHADER_H_

#include <memory>
#include <string>

#include <glm/fwd.hpp>

namespace twopi
{
namespace gl
{
class Shader
{
public:
  Shader() = delete;

  Shader(const std::string& vertex_shader_filepath, const std::string& fragment_shader_filepath);

  ~Shader();

  void Use();

  void Uniform1i(const std::string& name, int i);
  void Uniform1f(const std::string& name, float v);
  void Uniform3f(const std::string& name, const glm::vec3& v);
  void UniformMatrix3f(const std::string& name, const glm::mat3& m);
  void UniformMatrix4f(const std::string& name, const glm::mat4& m);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_GL_GL_SHADER_H_
