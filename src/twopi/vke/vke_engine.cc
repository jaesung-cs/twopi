#include <twopi/vke/vke_engine.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vulkan/vulkan.hpp>

#include <twopi/core/error.h>
#include <twopi/window/window.h>
#include <twopi/window/glfw_window.h>
#include <twopi/geometry/image_loader.h>
#include <twopi/geometry/image.h>
#include <twopi/geometry/mesh_loader.h>
#include <twopi/geometry/mesh.h>
#include <twopi/scene/camera.h>

#include <twopi/vke/vke_context.h>
#include <twopi/vke/vke_buffer.h>
#include <twopi/vke/vke_image.h>
#include <twopi/vke/vke_swapchain.h>

namespace twopi
{
namespace vke
{
class Engine::Impl
{
private:
  static constexpr int max_frames_in_flight_ = 2;
  static constexpr int alignment_ = 256;

public:
  Impl() = delete;

  Impl(std::shared_ptr<window::Window> window)
  {
    width_ = window->Width();
    height_ = window->Height();

    context_ = std::make_shared<vke::Context>(std::dynamic_pointer_cast<window::GlfwWindow>(window)->Handle());

    swapchain_ = std::make_shared<vke::Swapchain>(context_, width_, height_);

    // Load image
    const std::string mesh_filepath = "C:\\workspace\\twopi\\resources\\viking_room.obj";
    geometry::MeshLoader mesh_loader{};
    auto mesh = mesh_loader.Load(mesh_filepath);

    const auto& mesh_vertex_buffer = mesh->Vertices();
    const auto& mesh_normal_buffer = mesh->Normals();
    const auto& mesh_tex_coords_buffer = mesh->TexCoords();
    const auto mesh_buffer_size = (mesh_vertex_buffer.size() + mesh_normal_buffer.size() + mesh_tex_coords_buffer.size()) * sizeof(float);

    const auto& mesh_index_buffer = mesh->Indices();
    const auto mesh_index_buffer_size = mesh_index_buffer.size() * sizeof(uint32_t);

    constexpr int grid_size = 5;
    std::vector<glm::mat4> models;
    for (int x = 0; x < grid_size; x++)
    {
      for (int y = 0; y < grid_size; y++)
      {
        for (int z = 0; z < grid_size; z++)
        {
          glm::mat4 m = glm::mat4(1.f);
          m[3][0] = static_cast<float>(x);
          m[3][1] = static_cast<float>(y);
          m[3][2] = static_cast<float>(z);
          models.emplace_back(std::move(m));
        }
      }
    }

    const float* mesh_instance_buffer = glm::value_ptr(models[0]);
    const auto mesh_instance_buffer_size = models.size() * 16 * sizeof(float);

    vk::BufferCreateInfo buffer_create_info;
    buffer_create_info
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
      .setSize(mesh_buffer_size + mesh_index_buffer_size + mesh_instance_buffer_size);
    vertex_staging_buffer_ = std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Host);

    auto ptr = static_cast<unsigned char*>(vertex_staging_buffer_->Map());

    std::memcpy(ptr, mesh_vertex_buffer.data(), mesh_vertex_buffer.size() * sizeof(float));
    ptr += mesh_vertex_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_normal_buffer.data(), mesh_normal_buffer.size() * sizeof(float));
    ptr += mesh_normal_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_tex_coords_buffer.data(), mesh_tex_coords_buffer.size() * sizeof(float));
    ptr += mesh_tex_coords_buffer.size() * sizeof(float);

    std::memcpy(ptr, mesh_index_buffer.data(), mesh_index_buffer.size() * sizeof(uint32_t));
    ptr += mesh_index_buffer.size() * sizeof(uint32_t);

    std::memcpy(ptr, mesh_instance_buffer, mesh_instance_buffer_size);
    ptr += mesh_instance_buffer_size;

    vertex_staging_buffer_->Unmap();

    normal_offset_ = mesh_vertex_buffer.size() * sizeof(float);
    tex_coord_offset_ = normal_offset_ + mesh_normal_buffer.size() * sizeof(float);
    instance_offset_ = tex_coord_offset_ + mesh_tex_coords_buffer.size() * sizeof(float);
    num_indices_ = mesh_index_buffer.size();
    num_instances_ = models.size();

    buffer_create_info
      .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer)
      .setSize(mesh_buffer_size);
    vertex_buffer_ = std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Device);

    buffer_create_info
      .setSize(mesh_instance_buffer_size);
    instance_buffer_ = std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Device);

    buffer_create_info
      .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer)
      .setSize(mesh_index_buffer_size);
    index_buffer_ = std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Device);

    // Uniform buffers per swapchain images
    buffer_create_info
      .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer)
      .setSize(3 * 16 * sizeof(float));

    for (uint32_t i = 0; i < swapchain_->ImageCount(); i++)
      uniform_buffers_.emplace_back(std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Host));

    // Load image
    const std::string image_filepath = "C:\\workspace\\twopi\\resources\\viking_room.png";
    geometry::ImageLoader image_loader{};
    auto texture_image = image_loader.Load<uint8_t>(image_filepath);

    // Create vulkan texture image
    vk::ImageCreateInfo image_create_info;
    image_create_info
      .setImageType(vk::ImageType::e2D)
      .setSharingMode(vk::SharingMode::eExclusive)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setArrayLayers(1)
      .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
      .setFormat(vk::Format::eB8G8R8A8Srgb)
      .setMipLevels(3)
      .setExtent(vk::Extent3D{ texture_image->Width(), texture_image->Height(), 1 })
      .setSamples(vk::SampleCountFlagBits::e1);
    image_ = std::make_unique<vke::Image>(context_, image_create_info, vke::Image::MemoryType::Device);

    buffer_create_info
      .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
      .setSize(texture_image->Width() * texture_image->Height() * 4);
    image_staging_buffer_ = std::make_unique<vke::Buffer>(context_, buffer_create_info, vke::Buffer::MemoryType::Host);

    // Copy image pixels to image staging buffer
    auto* image_ptr = static_cast<uint8_t*>(image_staging_buffer_->Map());
    std::memcpy(image_ptr, texture_image->Buffer().data(), texture_image->Buffer().size());
    image_staging_buffer_->Unmap();
  }

  ~Impl()
  {
    vertex_staging_buffer_.reset();
    vertex_buffer_.reset();
    index_buffer_.reset();
    instance_buffer_.reset();

    image_.reset();
    image_staging_buffer_.reset();

    for (auto& uniform_buffer : uniform_buffers_)
      uniform_buffer.reset();
    uniform_buffers_.clear();

    swapchain_.reset();
    context_.reset();
  }

  void UpdateCamera(std::shared_ptr<scene::Camera> camera)
  {
    projection_matrix_ = camera->ProjectionMatrix();
    projection_matrix_[1][1] *= -1.f;

    view_matrix_ = camera->ViewMatrix();
  }

  void Draw()
  {
    std::cout << "TODO: Draw" << std::endl;

    // TODO: Acquire image index from swapchain
    const auto image_index = 0;

    // Update uniform buffer
    constexpr uint64_t mat4_size = sizeof(float) * 16;
    glm::mat4 model_matrix{ 1.f };

    auto* ptr = static_cast<unsigned char*>(uniform_buffers_[image_index]->Map());
    std::memcpy(ptr, glm::value_ptr(projection_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size, glm::value_ptr(view_matrix_), mat4_size);
    std::memcpy(ptr + mat4_size * 2, glm::value_ptr(model_matrix), mat4_size);
    uniform_buffers_[image_index]->Unmap();

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight_;
  }

  void Resize(int width, int height)
  {
    width_ = width;
    height_ = height;

    // TODO: Recreate swapchain
  }

private:
  // Context and memory management
  std::shared_ptr<vke::Context> context_;

  // Swapchain
  std::shared_ptr<vke::Swapchain> swapchain_;

  // Vertex attributes
  std::unique_ptr<vke::Buffer> vertex_staging_buffer_;
  std::unique_ptr<vke::Buffer> vertex_buffer_;
  std::unique_ptr<vke::Buffer> index_buffer_;
  std::unique_ptr<vke::Buffer> instance_buffer_;
  uint64_t normal_offset_ = 0;
  uint64_t tex_coord_offset_ = 0;
  uint64_t instance_offset_ = 0;
  uint64_t num_indices_ = 0;
  uint64_t num_instances_ = 0;

  // Texture
  std::unique_ptr<vke::Image> image_;
  std::unique_ptr<vke::Buffer> image_staging_buffer_;

  // Uniform buffer
  std::vector<std::unique_ptr<vke::Buffer>> uniform_buffers_;
  glm::mat4 projection_matrix_;
  glm::mat4 view_matrix_;

  // Rendering & presentation synchronization
  int current_frame_ = 0;

  // Window
  int width_ = 0;
  int height_ = 0;
};

Engine::Engine(std::shared_ptr<window::Window> window)
{
  impl_ = std::make_unique<Impl>(window);
}

Engine::~Engine() = default;

void Engine::Draw()
{
  impl_->Draw();
}

void Engine::Resize(int width, int height)
{
  impl_->Resize(width, height);
}

void Engine::UpdateCamera(std::shared_ptr<scene::Camera> camera)
{
  impl_->UpdateCamera(camera);
}
}
}
