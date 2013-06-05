#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "TurtleParser.hpp"
#include "DictionaryLoader.hpp"

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

  // Verify that file name is valid
  std::ifstream file(argv[1]);
  if (!file.good()) {
    return usageMessage(argv[0]);
  }

  std::cout << "Executing performance tests using Turtle data from '" << argv[1] << "'." << std::endl;

  // Load data from into all dictionaries in succession
  for (int i = 0; i < DictionaryLoader::DictionaryTypes; i++) {
    file.clear();
    file.seekg(0, std::ios_base::beg);

    DictionaryLoader::DictionaryType type  = static_cast<DictionaryLoader::DictionaryType>(i);

    std::cout << "Start loading into dictionary " << i << std::endl;

    TurtleParser parser(file);
    DictionaryLoader loader(type);
    parser.loadInto(loader);
    std::cout << "Done with dictionary " << i << std::endl;
  }

  file.close();

  std::cout << "Done." << std::endl;

  return 0;
}
