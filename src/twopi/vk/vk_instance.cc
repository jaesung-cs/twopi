#include <twopi/vk/vk_instance.h>

#include <iostream>

namespace twopi
{
namespace vk
{
std::vector<VkExtensionProperties> Instance::Extensions()
{
  uint32_t num_extensions = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr);
  std::vector<VkExtensionProperties> extensions(num_extensions);
  vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, extensions.data());
  return extensions;
}

class Instance::Impl
{
public:
  Impl()
  {
  }

  Impl(VkInstance instance)
    : instance_(std::make_shared<Handle>(instance))
  {
  }

  ~Impl()
  {
  }

  operator VkInstance() const { return *instance_; }

private:
  class Handle
  {
  public:
    Handle(VkInstance instance) : instance_(instance) {}

    ~Handle()
    {
      vkDestroyInstance(instance_, nullptr);
    }

    operator VkInstance() const { return instance_; }

  private:
    const VkInstance instance_;
  };

  std::shared_ptr<Handle> instance_;
};

Instance::Instance()
  : impl_(std::make_unique<Impl>())
{
}

Instance::Instance(VkInstance instance)
  : impl_(std::make_unique<Impl>(instance))
{
}

Instance::Instance(const Instance& rhs)
  : impl_(std::make_unique<Impl>(*rhs.impl_))
{
}

Instance& Instance::operator = (const Instance& rhs)
{
  impl_ = std::make_unique<Impl>(*rhs.impl_);
  return *this;
}

Instance::Instance(Instance&& rhs) = default;

Instance& Instance::operator = (Instance&& rhs) = default;

Instance::~Instance() = default;
}
}
