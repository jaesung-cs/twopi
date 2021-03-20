#include <iostream>

#include <twopi/application/application.h>

int main()
{
  twopi::Application app;

  try
  {
    app.Run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
