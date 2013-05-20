#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "TurtleParser.hpp"
#include "DictionaryLoader.hpp"

/**
 * @file
 * Load data from a turtle file into a string dictionary
 *
 * Allows for different string dictionary implementations
 * and pauses after loading to allow for memory usage
 * measurements.
 */

static constexpr const char* typeStrings[] = {
  "dummy",
  "uncompressed",
  "simple",
  "art",
  "hash-art",
};

inline int usageMessage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [dictionary implementation] [turtle file]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Available dictionary implementations:" << std::endl;
  for (auto type : typeStrings) {
    std::cerr << " > " << type << std::endl;
  }
  return 1;
}

inline bool getDictionaryType(std::string typeString, DictionaryLoader::DictionaryType& type) {
  for (size_t i = 0; i < sizeof(typeStrings)/sizeof(char*); i++) {
    if (typeStrings[i] == typeString) {
      type = static_cast<DictionaryLoader::DictionaryType>(i);
      return true;
    }
  }
  return false;
}

int main(int argc, const char** argv) {
  if (argc != 3) {
    return usageMessage(argv[0]);
  }

  // Verify that dictionary type is valid
  DictionaryLoader::DictionaryType type;
  if (!getDictionaryType(argv[1], type)) {
    return usageMessage(argv[0]);
  }

  // Verify that file name is valid
  std::ifstream file(argv[2]);
  if (!file.good()) {
    return usageMessage(argv[0]);
  }

  std::cout << "Reading Turtle data from '" << argv[2] << "' into " << argv[1] << " dictionary." << std::endl;

  TurtleParser parser(file);
  DictionaryLoader loader(type);
  parser.loadInto(loader);
  file.close();

  std::cout << "Done." << std::endl;
  std::cout << "Press Enter to exit...";
  std::cin.get();

  return 0;
}
