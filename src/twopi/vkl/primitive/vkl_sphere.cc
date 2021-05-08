#include <twopi/vkl/primitive/vkl_sphere.h>

#include <cmath>

#include <glm/ext.hpp>

namespace twopi
{
namespace vkl
{
Sphere::Sphere(int segments)
{
  const uint32_t top_index = segments * (segments - 1);
  const uint32_t bottom_index = top_index + 1;
  for (int i = 0; i < segments; i++)
  {
    index_buffer_.push_back(top_index);
    index_buffer_.push_back(i * (segments - 1));
    index_buffer_.push_back(((i + 1) % segments) * (segments - 1));

    const float u = static_cast<float>(i) / segments;
    const float theta = u * 2.f * glm::pi<float>();
    for (int j = 1; j < segments; j++)
    {
      const float v = static_cast<float>(j) / segments;
      const float phi = v * glm::pi<float>();
      position_buffer_.push_back(std::cos(theta) * std::sin(phi));
      position_buffer_.push_back(std::sin(theta) * std::sin(phi));
      position_buffer_.push_back(std::cos(phi));
      normal_buffer_.push_back(std::cos(theta) * std::sin(phi));
      normal_buffer_.push_back(std::sin(theta) * std::sin(phi));
      normal_buffer_.push_back(std::cos(phi));

      if (j < segments - 1)
      {
        index_buffer_.push_back(i * (segments - 1) + j - 1);
        index_buffer_.push_back(i * (segments - 1) + j);
        index_buffer_.push_back(((i + 1) % segments) * (segments - 1) + j - 1);

        index_buffer_.push_back(i * (segments - 1) + j);
        index_buffer_.push_back(((i + 1) % segments) * (segments - 1) + j);
        index_buffer_.push_back(((i + 1) % segments) * (segments - 1) + j - 1);
      }
    }

    index_buffer_.push_back(i * (segments - 1) + segments - 2);
    index_buffer_.push_back(bottom_index);
    index_buffer_.push_back(((i + 1) % segments) * (segments - 1) + segments - 2);
  }

  position_buffer_.push_back(0.f);
  position_buffer_.push_back(0.f);
  position_buffer_.push_back(1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(1.f);

  position_buffer_.push_back(0.f);
  position_buffer_.push_back(0.f);
  position_buffer_.push_back(-1.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(0.f);
  normal_buffer_.push_back(-1.f);
}

Sphere::~Sphere() = default;
}
}
