#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
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
  //TODO: generally implement this in a nicer way
  for (int dictId = 0; dictId < DictionaryLoader::DictionaryTypes; dictId++) {
    std::cout << "Base memory usage:" << std::endl;
    system(("ps -p " + std::to_string(getpid()) + " -orss=").c_str());

    int status;
    if (fork() == 0) {
      DictionaryLoader loader(static_cast<DictionaryLoader::DictionaryType>(dictId));
      Dictionary* dict = loader.getDictionary();

      std::cout << "Start loading into " << dict << "." << std::endl;
      clock_t startClock = clock();

      // TODO: Bulk load part of data, rest later
      TurtleParser parser(file);
      parser.loadInto(loader);
      clock_t ticks = clock() - startClock;
      std::cout << "Data import finished." << std::endl;
      std::cout << "Took " << ticks << " ticks." << std::endl;
      std::cout << "Memory usage:" << std::endl;
      system(("ps -p " + std::to_string(getpid()) + " -orss=").c_str());
      std::cout << "Subtract base memory usage" << std::endl;

      uint64_t operations = static_cast<uint64_t>(0.2 * dict->size());

      std::cout << "Generating random IDs" << std::endl;
      std::random_device device;
      std::mt19937_64 engine(device());
      std::uniform_int_distribution<uint64_t> dist(0, loader.getDictionary()->size()-1);

      uint64_t* identifiers = new uint64_t[operations];
      for (uint64_t i = 0; i < operations; i++) {
        identifiers[i] = dist(engine);
      }

      std::cout << "Now looking up " << operations << " random triples by ID." << std::endl;
      std::string* values = new std::string[operations];
      startClock = clock();
      for (uint64_t i = 0; i < operations; i++) {
        assert(dict->lookup(identifiers[i], values[i]));
      }
      ticks = clock() - startClock;
      std::cout << "Took " << ticks << " ticks." << std::endl;

      std::cout << "Now looking up " << operations << " random triples by name." << std::endl;
      startClock = clock();
      for (uint64_t i = 0; i < operations; i++) {
        uint64_t id;
        assert(dict->lookup(values[i], id));
      }
      ticks = clock() - startClock;
      std::cout << "Took " << ticks << " ticks." << std::endl;

      std::cout << "Now updating " << operations << " random triples." << std::endl;
      startClock = clock();
      for (uint64_t i = 0; i < operations; i++) {
        assert(dict->update(identifiers[i], values[operations-i]));
      }
      ticks = clock() - startClock;
      std::cout << "Took " << ticks << " ticks." << std::endl;

      delete[] values;
      delete[] identifiers;
      std::cout << "---" << std::endl;
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
