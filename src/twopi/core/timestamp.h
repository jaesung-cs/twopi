#ifndef TWOPI_CORE_TIMESTAMP_H_
#define TWOPI_CORE_TIMESTAMP_H_

#include <chrono>

namespace twopi
{
namespace core
{
using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
}
}

#endif // TWOPI_CORE_TIMESTAMP_H_
