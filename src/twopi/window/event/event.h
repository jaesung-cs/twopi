#ifndef TWOPI_WINDOW_EVENT_EVENT_H_
#define TWOPI_WINDOW_EVENT_EVENT_H_

#include <memory>
#include <cstdint>

#include <twopi/core/timestamp.h>

namespace twopi
{
namespace window
{
class Event
{
public:
  Event();
  virtual ~Event();

  void SetTimestamp(core::Timestamp timestamp);

  core::Timestamp Timestamp() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
}

#endif // TWOPI_WINDOW_EVENT_EVENT_H_
