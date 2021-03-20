#ifndef TWOPI_CORE_ERROR_H_
#define TWOPI_CORE_ERROR_H_

#include <string>
#include <exception>

namespace twopi
{
namespace core
{
class Error : public std::exception
{
public:
  Error(const std::string& message)
    : std::exception(message.c_str())
  {
  }

  ~Error() = default;

private:
};
}
}

#endif // TWOPI_CORE_ERROR_H_
