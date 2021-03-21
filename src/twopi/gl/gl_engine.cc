#include <twopi/gl/gl_engine.h>

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <twopi/core/error.h>
#include <twopi/gl/gl_shader.h>
#include <twopi/gl/gl_geometry.h>
#include <twopi/gl/gl_texture.h>
#include <twopi/gl/gl_framebuffer.h>
#include <twopi/gl/gl_renderbuffer.h>
#include <twopi/scene/camera.h>
#include <twopi/scene/vr_camera.h>
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
    mesh_shader_ = std::make_shared<Shader>(dirpath + "mesh.vert", dirpath + "mesh.frag");
    mesh_shader_->Uniform1i("tex_sampler", 0);

    screen_shader_ = std::make_shared<Shader>(dirpath + "screen.vert", dirpath + "screen.frag");
    screen_shader_->Uniform1i("tex_sampler", 0);

    vr_screen_shader_ = std::make_shared<Shader>(dirpath + "vr_screen.vert", dirpath + "vr_screen.frag");
    vr_screen_shader_->Uniform1i("tex_sampler_left", 0);
    vr_screen_shader_->Uniform1i("tex_sampler_right", 1);

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

    screen_geometry_ = std::make_shared<Geometry>();
    screen_geometry_->SetAttribute(0, 2, {
      0.f, 0.f,
      1.f, 0.f,
      0.f, 1.f,
      1.f, 1.f,
    });
    screen_geometry_->SetElements({ 0, 1, 2, 1, 3, 2 });
    screen_geometry_->SetTriangles();
  }

  ~EngineImpl() = default;

  void SetViewport(int x, int y, int width, int height)
  {
    glViewport(x, y, width, height);

    width_ = width;
    height_ = height;

    // Framebuffers for eyes
    for (int i = 0; i < 2; i++)
    {
      screen_depth_renderbuffers_multisample_[i] = std::make_shared<Renderbuffer>();
      screen_depth_renderbuffers_multisample_[i]->DepthStencilStorageMultisample(width, height);

      screen_color_textures_multisample_[i] = std::make_shared<Texture>();
      screen_color_textures_multisample_[i]->StorageMultisample(width, height);

      screen_framebuffers_multisample_[i] = std::make_shared<Framebuffer>(width, height);
      screen_framebuffers_multisample_[i]->AttachColor(0, screen_color_textures_multisample_[i]);
      screen_framebuffers_multisample_[i]->AttachDepthStencil(screen_depth_renderbuffers_multisample_[i]);

      screen_color_textures_[i] = std::make_shared<Texture>();
      screen_color_textures_[i]->Storage(width, height);
    }

    screen_framebuffer_ = std::make_shared<Framebuffer>(width, height);
    screen_framebuffer_->AttachColor(0, screen_color_textures_[0]);
    screen_framebuffer_->AttachColor(1, screen_color_textures_[1]);
  } 

  void Draw()
  {
    if (!vr_mode_)
    {
      mesh_shader_->UniformMatrix4f("projection", projection_matrix_);
      mesh_shader_->UniformMatrix4f("view", view_matrices_[0]);
      mesh_shader_->UniformMatrix4f("model", glm::mat4(1.f));

      screen_framebuffers_multisample_[0]->Bind();

      glClearColor(0.8f, 0.8f, 0.8f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      mesh_shader_->Use();
      mesh_texture_->Bind(0);
      mesh_geometry_->Draw();

      screen_framebuffers_multisample_[0]->BlitTo(screen_framebuffer_);

      Framebuffer::Unbind();

      glClearColor(0.8f, 0.8f, 0.8f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      screen_shader_->Use();
      screen_color_textures_[0]->Bind(0);
      screen_geometry_->Draw();
    }
    else
    {
      mesh_shader_->UniformMatrix4f("projection", projection_matrix_);
      mesh_shader_->UniformMatrix4f("model", glm::mat4(1.f));

      for (int i = 0; i < 2; i++)
      {
        mesh_shader_->UniformMatrix4f("view", view_matrices_[i]);

        screen_framebuffers_multisample_[i]->Bind();

        glClearColor(0.8f, 0.8f, 0.8f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mesh_shader_->Use();
        mesh_texture_->Bind(0);
        mesh_geometry_->Draw();

        screen_framebuffers_multisample_[i]->BlitTo(0, screen_framebuffer_, i);
      }

      Framebuffer::Unbind();

      glClearColor(0.8f, 0.8f, 0.8f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      vr_screen_shader_->Use();
      screen_color_textures_[0]->Bind(0);
      screen_color_textures_[1]->Bind(1);
      screen_geometry_->Draw();
    }
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    projection_matrix_ = camera->ProjectionMatrix();

    if (auto vr_camera = std::dynamic_pointer_cast<scene::VrCamera>(camera))
    {
      vr_mode_ = true;
      view_matrices_[0] = vr_camera->ViewMatrix(0);
      view_matrices_[1] = vr_camera->ViewMatrix(1);
    }
    else
    {
      vr_mode_ = false;
      view_matrices_[0] = camera->ViewMatrix();
    }
  }

private:
  int width_ = 1;
  int height_ = 1;

  bool vr_mode_ = false;

  glm::mat4 projection_matrix_;
  glm::mat4 view_matrices_[2];
  
  std::shared_ptr<Shader> mesh_shader_;
  std::shared_ptr<Geometry> mesh_geometry_;
  std::shared_ptr<Texture> mesh_texture_;

  std::shared_ptr<Shader> screen_shader_;
  std::shared_ptr<Shader> vr_screen_shader_;
  std::shared_ptr<Geometry> screen_geometry_;

  std::shared_ptr<Framebuffer> screen_framebuffers_multisample_[2];
  std::shared_ptr<Texture> screen_color_textures_multisample_[2];
  std::shared_ptr<Renderbuffer> screen_depth_renderbuffers_multisample_[2];

  std::shared_ptr<Framebuffer> screen_framebuffer_;
  std::shared_ptr<Texture> screen_color_textures_[2];
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
