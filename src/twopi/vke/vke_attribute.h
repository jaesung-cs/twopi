#ifndef TWOPI_VKE_VKE_ATTRIBUTE_H_
#define TWOPI_VKE_VKE_ATTRIBUTE_H_

namespace twopi
{
namespace vke
{
struct Attribute
{
  enum class Type
  {
    VERTEX,
    INSTANCE,
  };

  Attribute() = delete;

  Attribute(int location, int size, Type type = Type::VERTEX)
    : type(type), location(location), rows(size) {}

  Attribute(int location, int rows, int cols, Type type = Type::VERTEX)
    : type(type), location(location), rows(rows), cols(cols) {}

  Type type;
  int location;
  int rows;
  int cols;
};
}
}

#endif // TWOPI_VKE_VKE_ATTRIBUTE_H_
