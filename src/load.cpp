#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "TurtleParser.hpp"

#include "UncompressedDictionary.hpp"
#include "DummyDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "ARTDictionary.hpp"
#include "HashARTDictionary.hpp"

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

inline Dictionary* getDictionary(std::string name) {
  if (name == typeStrings[0]) return new DummyDictionary();
  if (name == typeStrings[1]) return new UncompressedDictionary();
  if (name == typeStrings[2]) return new SimpleDictionary();
  if (name == typeStrings[3]) return new ARTDictionary();
  if (name == typeStrings[4]) return new HashARTDictionary();

  return nullptr;
}

int main(int argc, const char** argv) {
  if (argc != 3) {
    return usageMessage(argv[0]);
  }

  // Verify that dictionary type is valid
  Dictionary* dict = getDictionary(argv[1]);
  if (dict == nullptr) {
    return usageMessage(argv[0]);
  }

  // Verify that file name is valid
  std::ifstream file(argv[2]);
  if (!file.good()) {
    return usageMessage(argv[0]);
  }

  std::cout << "Reading Turtle data from '" << argv[2] << "' into " << argv[1] << " dictionary." << std::endl;

  TurtleParser parser(file);
  parser.loadInto(dict);
  file.close();

  delete dict;

  std::cout << "Done." << std::endl;
  std::cout << "Press Enter to exit...";
  std::cin.get();

  return 0;
}
