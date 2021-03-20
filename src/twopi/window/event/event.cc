#include <twopi/window/event/event.h>

namespace twopi
{
namespace window
{
namespace impl
{
class EventImpl
{
public:
  EventImpl()
  {
    timestamp_ = core::Clock::now();
  }

  ~EventImpl() = default;

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
}

Event::Event()
{
  impl_ = std::make_unique<impl::EventImpl>();
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
