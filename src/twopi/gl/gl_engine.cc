#include <twopi/gl/gl_engine.h>

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <twopi/core/error.h>
#include <twopi/gl/gl_shader.h>
#include <twopi/gl/gl_geometry.h>
#include <twopi/scene/camera.h>
#include <twopi/geometry/mesh.h>
#include <twopi/geometry/mesh_loader.h>

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

    // TODO: path
    const std::string dirpath = "C:\\workspace\\twopi\\src\\twopi\\shader\\";
    color_shader_ = std::make_shared<Shader>(dirpath + "color.vert", dirpath + "color.frag");

    // Load mesh
    // TODO: path
    const std::string mesh_filepath = "C:\\workspace\\twopi\\resources\\viking_room.obj";
    geometry::MeshLoader mesh_loader;
    const auto mesh = mesh_loader.Load(mesh_filepath);

    mesh_geometry_ = std::make_shared<Geometry>();
    mesh_geometry_->SetAttribute(0, 3, mesh->Vertices());
    mesh_geometry_->SetAttribute(1, 3, mesh->Normals());
    mesh_geometry_->SetElements(mesh->Indices());
    mesh_geometry_->SetTriangles();
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

    color_shader_->Use();
    mesh_geometry_->Draw();
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    color_shader_->UniformMatrix4f("projection", camera->ProjectionMatrix());
    color_shader_->UniformMatrix4f("view", camera->ViewMatrix());
    color_shader_->UniformMatrix4f("model", glm::mat4(1.f));
  }

private:
  std::shared_ptr<Shader> color_shader_;
  std::shared_ptr<Geometry> mesh_geometry_;
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
