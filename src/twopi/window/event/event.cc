#include <twopi/window/event/event.h>

namespace twopi
{
namespace window
{
class Event::Impl
{
public:
  Impl()
  {
    timestamp_ = core::Clock::now();
  }

  ~Impl() = default;

  void SetTimestamp(core::Timestamp timestamp)
  {
    timestamp_ = timestamp;
  }

  core::Timestamp Timestamp() const
  {
    return timestamp_;
  }

private:
  core::Timestamp timestamp_;
};

Event::Event()
{
  impl_ = std::make_unique<Impl>();
}

Event::~Event() = default;

void Event::SetTimestamp(core::Timestamp timestamp)
{
  impl_->SetTimestamp(timestamp);
}

core::Timestamp Event::Timestamp() const
{
  return impl_->Timestamp();
}
}
}
