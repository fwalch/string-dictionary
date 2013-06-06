#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include "TurtleParser.hpp"
#include "DictionaryLoader.hpp"

/**
 * @file
 * Executes performance tests for dictionary implementations.
 */

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);
inline int usageMessage(const char* argv0);

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

  //TODO: automate memory usage by reading /proc
  // Load data from into all dictionaries in succession
  for (int i = 0; i < DictionaryLoader::DictionaryTypes; i++) {
    std::cout << "Base memory usage:" << std::endl;
    system(("ps -p " + std::to_string(getpid()) + " -orss=").c_str());

    int status;
    if (fork() == 0) {
      DictionaryLoader loader(static_cast<DictionaryLoader::DictionaryType>(i));

      std::cout << "Start loading into " << loader.getDictionary() << "." << std::endl;
      clock_t startClock = clock();

      TurtleParser parser(file);
      parser.loadInto(loader);
      clock_t ticks = clock() - startClock;
      std::cout << "Data import finished." << std::endl;
      std::cout << "Took " << ticks << " ticks." << std::endl;
      std::cout << "Memory usage:" << std::endl;
      system(("ps -p " + std::to_string(getpid()) + " -orss=").c_str());
      std::cout << "Subtract base memory usage" << std::endl;
      exit(0);
    }
    else {
      wait(&status);
    }

    file.clear();
    file.seekg(0, std::ios_base::beg);
  }

  file.close();

  std::cout << "Done." << std::endl;

  return 0;
}

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict) {
    return stream << dict->name();
}

inline int usageMessage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [turtle file]" << std::endl;
  return 1;
}
