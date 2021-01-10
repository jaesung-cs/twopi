#include <twopi/application/application.h>

#include <twopi/application/application_vk.h>

namespace twopi
{
Application::Application()
{
  pimpl_ = std::make_unique<impl::ApplicationVk>();
}

Application::~Application() = default;

void Application::Run()
{
  pimpl_->Run();
}
}
