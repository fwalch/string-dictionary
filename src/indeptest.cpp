#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include "ArtLeafRunner.hpp"

/**
 * @file
 * Executes performance tests for dictionary implementations.
 */

inline int usageMessage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [turtle file]" << std::endl;
  return 1;
}

int main(int argc, const char** argv) {
  if (argc != 2) {
    return usageMessage(argv[0]);
  }

  std::ifstream file(argv[1]);
  if (!file.good()) {
    return usageMessage(argv[0]);
  }

  std::cout << "Executing performance tests using Turtle data from '" << argv[1] << "'." << std::endl;

  ArtLeafRunner testRunner;
  testRunner.run(file);
  file.close();

  std::cout << "Performance tests finished." << std::endl;

  return 0;
}
