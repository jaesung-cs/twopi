#ifndef TWOPI_WINDOW_EVENT_EVENT_H_
#define TWOPI_WINDOW_EVENT_EVENT_H_

#include <memory>
#include <cstdint>

#include <twopi/core/timestamp.h>

namespace twopi
{
namespace window
{
namespace impl
{
class EventImpl;
}

class Event
{
public:
  Event();
  virtual ~Event();

  void SetTimestamp(core::Timestamp timestamp);

  core::Timestamp Timestamp() const;

private:
  std::unique_ptr<impl::EventImpl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_EVENT_H_
