// This file is designed to fail CI on purpose
#include <iostream>

int main() {
  if (true) { // <-- This should fail clang-format (bad spacing)
    int *p = 0;
    int x = *p; // <-- This should fail clang-tidy (null pointer dereference)
    std::cout << "Test" << x << std::endl;
  }
  return 0;
}
