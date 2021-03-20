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
    : std::exception()
    , message_(message)
  {
  }

  ~Error() = default;

  virtual const char* what() const noexcept
  {
    return message_.c_str();
  }

private:
  std::string message_;
};
}
}

#endif // TWOPI_CORE_ERROR_H_
