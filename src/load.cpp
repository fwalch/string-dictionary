#include <iostream>
#include <fstream>
#include <exception>
#include <set>
#include <string>
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

inline int usageMessage(std::string argv0, std::set<std::string> typeStrings) {
  std::cerr << "Usage: " << argv0 << " [dictionary type] [turtle file]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Available dictionary types:" << std::endl;
  for (std::string type : typeStrings) {
    std::cerr << " > " << type << std::endl;
  }
  return 1;
}

inline bool getDictionaryType(std::set<std::string> typeStrings, std::string typeString, DictionaryLoader::DictionaryType& type) {
  auto it = typeStrings.find(typeString);
  if (it == typeStrings.end()) {
    return false;
  }
  //TODO: this can be done more easily, right?
  type = static_cast<DictionaryLoader::DictionaryType>(static_cast<unsigned long>(&(*it) - &(*typeStrings.begin()))/sizeof(DictionaryLoader::DictionaryType));
  return true;
}

int main(int argc, const char** argv) {
  //TODO: move to global variable
  std::set<std::string> typeStrings = {
    "dummy",
    "uncompressed"
  };

  if (argc != 3) {
    return usageMessage(argv[0], typeStrings);
  }

  DictionaryLoader::DictionaryType type;
  if (!getDictionaryType(typeStrings, argv[1], type)) {
    return usageMessage(argv[0], typeStrings);
  }

  std::ifstream file(argv[2]);
  if (!file.good()) {
    return usageMessage(argv[0], typeStrings);
  }

  std::cout << "Reading Turtle data from '" << argv[2] << "' into " << argv[1] << " dictionary." << std::endl;
  TurtleParser parser(file);
  DictionaryLoader loader(type);

  parser.loadInto(loader);

  file.close();

  // Wait until exit
  std::cout << "Press any key to exit..." << std::endl;
  std::cin.get();

  return 0;
}
