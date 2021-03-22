#include <twopi/gl/gl_shader.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <twopi/core/error.h>

namespace twopi
{
namespace gl
{
class Shader::Impl
{
public:
  Impl(const std::string& vertex_shader_filepath, const std::string& fragment_shader_filepath)
  {
    GLuint vertex_shader = CreateShader(vertex_shader_filepath, GL_VERTEX_SHADER);
    GLuint fragment_shader = CreateShader(fragment_shader_filepath, GL_FRAGMENT_SHADER);

    program_ = CreateProgram({ vertex_shader, fragment_shader });

    DeleteShader(vertex_shader);
    DeleteShader(fragment_shader);
  }

  ~Impl()
  {
    glDeleteProgram(program_);
  }

  void Use()
  {
    glUseProgram(program_);
  }

  void Uniform1i(const std::string& name, int i)
  {
    glProgramUniform1i(program_, glGetUniformLocation(program_, name.c_str()), i);
  }

  void Uniform1f(const std::string& name, float v)
  {
    glProgramUniform1f(program_, glGetUniformLocation(program_, name.c_str()), v);
  }

  void Uniform3f(const std::string& name, const glm::vec3& v)
  {
    glProgramUniform3fv(program_, glGetUniformLocation(program_, name.c_str()), 1, glm::value_ptr(v));
  }

  void UniformMatrix3f(const std::string& name, const glm::mat3& m)
  {
    glProgramUniformMatrix3fv(program_, glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
  }

  void UniformMatrix4f(const std::string& name, const glm::mat4& m)
  {
    glProgramUniformMatrix4fv(program_, glGetUniformLocation(program_, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
  }

private:
  GLuint CreateShader(const std::string& filepath, GLenum type)
  {
    auto shader = glCreateShader(type);

    std::string code;
    std::ifstream file;
    file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
      file.open(filepath);
      std::stringstream ss;
      ss << file.rdbuf();
      file.close();
      code = ss.str();
    }
    catch (const std::ifstream::failure& e)
    {
      throw core::Error("Failed to load shader from file: " + filepath + "\nmessage: " + e.what());
    }

    const char* code_ptr = code.c_str();

    glShaderSource(shader, 1, &code_ptr, NULL);
    glCompileShader(shader);

    int success;
    char info_log[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(shader, 1024, NULL, info_log);
      throw core::Error(info_log);
    }

    return shader;
  }

  void DeleteShader(GLuint shader)
  {
    glDeleteShader(shader);
  }

  GLuint CreateProgram(const std::vector<GLuint>& shaders)
  {
    auto program = glCreateProgram();

    for (auto shader : shaders)
    {
      glAttachShader(program, shader);
    }
    
    glLinkProgram(program);

    int success;
    char info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(program, 1024, NULL, info_log);
      throw core::Error(info_log);
    }

    for (auto shader : shaders)
    {
      glDetachShader(program, shader);
    }

    return program;
  }

  GLuint program_ = 0;
};

Shader::Shader(const std::string& vertex_shader_filepath, const std::string& fragment_shader_filepath)
{
  impl_ = std::make_unique<Impl>(vertex_shader_filepath, fragment_shader_filepath);
}

Shader::~Shader()
{
}

void Shader::Use()
{
  impl_->Use();
}

void Shader::Uniform1i(const std::string& name, int i)
{
  impl_->Uniform1i(name, i);
}

void Shader::Uniform1f(const std::string& name, float v)
{
  impl_->Uniform1f(name, v);
}

void Shader::Uniform3f(const std::string& name, const glm::vec3& v)
{
  impl_->Uniform3f(name, v);
}

void Shader::UniformMatrix3f(const std::string& name, const glm::mat3& m)
{
  impl_->UniformMatrix3f(name, m);
}

void Shader::UniformMatrix4f(const std::string& name, const glm::mat4& m)
{
  impl_->UniformMatrix4f(name, m);
}
}
}
