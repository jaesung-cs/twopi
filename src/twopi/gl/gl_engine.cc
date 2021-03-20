#include <twopi/gl/gl_engine.h>

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <twopi/core/error.h>
#include <twopi/gl/gl_shader.h>
#include <twopi/gl/gl_geometry.h>
#include <twopi/gl/gl_texture.h>
#include <twopi/scene/camera.h>
#include <twopi/geometry/mesh.h>
#include <twopi/geometry/mesh_loader.h>
#include <twopi/geometry/image.h>
#include <twopi/geometry/image_loader.h>

namespace twopi
{
namespace gl
{
namespace impl
{
class EngineImpl
{
public:
  EngineImpl()
  {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      throw core::Error("Failed to load GL loader via glad.");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // TODO: shader path
    const std::string dirpath = "C:\\workspace\\twopi\\src\\twopi\\shader\\";
    color_shader_ = std::make_shared<Shader>(dirpath + "color.vert", dirpath + "color.frag");
    mesh_shader_ = std::make_shared<Shader>(dirpath + "mesh.vert", dirpath + "mesh.frag");

    // Load mesh
    // TODO: mesh path
    const std::string mesh_filepath = "C:\\workspace\\twopi\\resources\\viking_room.obj";
    geometry::MeshLoader mesh_loader;
    const auto mesh = mesh_loader.Load(mesh_filepath);

    mesh_geometry_ = std::make_shared<Geometry>();
    mesh_geometry_->SetAttribute(0, 3, mesh->Vertices());
    mesh_geometry_->SetAttribute(1, 3, mesh->Normals());
    mesh_geometry_->SetAttribute(2, 2, mesh->TexCoords());
    mesh_geometry_->SetElements(mesh->Indices());
    mesh_geometry_->SetTriangles();

    // TODO: texture filepath from mesh
    const auto texture_filepath = "C:\\workspace\\twopi\\resources\\viking_room.png";
    geometry::ImageLoader image_laoder;
    const auto image = image_laoder.Load<uint8_t>(texture_filepath);

    mesh_texture_ = std::make_shared<Texture>();
    mesh_texture_->Load(image);
  }

  ~EngineImpl() = default;

  void SetViewport(int x, int y, int width, int height)
  {
    glViewport(x, y, width, height);
  } 

  void Draw()
  {
    glClearColor(0.8f, 0.8f, 0.8f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mesh_shader_->Use();
    mesh_texture_->Bind(0);
    mesh_geometry_->Draw();
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    color_shader_->UniformMatrix4f("projection", camera->ProjectionMatrix());
    color_shader_->UniformMatrix4f("view", camera->ViewMatrix());
    color_shader_->UniformMatrix4f("model", glm::mat4(1.f));

    mesh_shader_->UniformMatrix4f("projection", camera->ProjectionMatrix());
    mesh_shader_->UniformMatrix4f("view", camera->ViewMatrix());
    mesh_shader_->UniformMatrix4f("model", glm::mat4(1.f));
  }

private:
  std::shared_ptr<Shader> color_shader_;
  std::shared_ptr<Shader> mesh_shader_;
  std::shared_ptr<Geometry> mesh_geometry_;
  std::shared_ptr<Texture> mesh_texture_;
};
}

Engine::Engine()
{
  impl_ = std::make_unique<impl::EngineImpl>();
}

Engine::~Engine() = default;

void Engine::SetViewport(int x, int y, int width, int height)
{
  impl_->SetViewport(x, y, width, height);
}

void Engine::Draw()
{
  impl_->Draw();
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
