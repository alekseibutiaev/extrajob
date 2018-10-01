#include <iostream>
#include <exception>

int main(int ac, char* av[]) {
  try {
    return 0;
  }
  catch(const std::exception& e) {
    std::cout << "error: " << e.what() << std::endl;
  }
  return 1;
}
