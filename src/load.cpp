#include <iostream>
#include <fstream>
#include <exception>
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

inline int usageMessage(const char* argv0) {
   std::cerr << "Usage: " << argv0 << " [turtle file]" << std::endl;
   return 1;
}

int main(int argc, char** argv) {
   if (argc != 2) {
     return usageMessage(argv[0]);
   }

   std::ifstream file(argv[1]);
   if (!file.good()) {
     return usageMessage(argv[0]);
   }

   std::cout << "Reading Turtle data from '" << argv[1] << "'" << std::endl;
   TurtleParser parser(file);
   DictionaryLoader loader;

   parser.loadInto(loader);

   // Wait until exit
   std::cout << "Press any key to exit..." << std::endl;
   std::cin.get();

   return 0;
}
