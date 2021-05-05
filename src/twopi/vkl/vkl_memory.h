#ifndef TWOPI_VKL_VKL_MEMORY_H_
#define TWOPI_VKL_VKL_MEMORY_H_

#include <vulkan/vulkan.hpp>

namespace twopi
{
namespace vkl
{
struct Memory
{
  vk::DeviceMemory device_memory;
  vk::DeviceSize offset = 0;
  vk::DeviceSize size = 0;
};
}
}

#endif // TWOPI_VKL_VKL_MEMORY_H_
