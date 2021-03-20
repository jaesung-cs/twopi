#include <twopi/application/application.h>

#include <iostream>
#include <exception>

int main()
{
  try
  {
    twopi::app::Application app;
    app.Run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
