#include <cassert>
#include "PerformanceTestRunner.hpp"
#include "TurtleParser.hpp"
#include "Dictionaries.hpp"
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

inline bool hasDictionary(char counter);
inline Dictionary* getDictionary(char counter);
inline uint64_t getNumberOfValues(istream& tupleStream);
inline uint64_t* getRandomIDs(uint64_t numberOfOperations, uint64_t numberOfValues);
inline void resetStream(istream& tupleStream);
inline float diff(clock_t start);

void PerformanceTestRunner::run(istream& tupleStream) {
  uint64_t numberOfValues = getNumberOfValues(tupleStream);
  cout << "Turtle file contains " << numberOfValues << " string values." << endl;

  cout << "---" << endl;

  uint64_t numberOfOperations = static_cast<uint64_t>(0.3 * numberOfValues);

  //TODO: automate memory usage by reading /proc/*/smaps
  // Load data from into all dictionaries in succession
  //TODO: generally implement this in a nicer way
  for (char counter = 0; hasDictionary(counter); counter++) {
    int status;
    if (fork() == 0) {
      Dictionary* dict = getDictionary(counter);

      clock_t start;

      std::cout << "Inserting " << numberOfValues << " strings into " << dict << "." << std::endl;
      start = clock();
      TurtleParser parser(tupleStream);
      parser.loadInto(dict);
      std::cout << "Finished in " << diff(start) << " sec." << std::endl;

      std::cout << "Memory usage:" << std::endl;
      system(("ps -p " + std::to_string(getpid()) + " -orss=").c_str());

      uint64_t* identifiers = getRandomIDs(numberOfOperations, dict->size());

      std::cout << "Looking up " << numberOfOperations << " strings by ID." << std::endl;
      std::string* values = new std::string[numberOfOperations];
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
        assert(dict->lookup(identifiers[i], values[i]));
      }
      std::cout << "Finished in " << diff(start) << " sec." << std::endl;

      std::cout << "Looking up " << numberOfOperations << " strings by value." << std::endl;
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
        uint64_t id;
#pragma GCC diagnostic pop
        assert(dict->lookup(values[i], id));
      }
      std::cout << "Finished in " << diff(start) << " sec." << std::endl;

      std::cout << "Updating " << numberOfOperations << " strings." << std::endl;
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
        assert(dict->update(identifiers[i], values[numberOfOperations-1-i]));
      }
      std::cout << "Finished in " << diff(start) << " sec." << std::endl;

      delete[] values;
      delete[] identifiers;
      delete dict;
      std::cout << "---" << std::endl;
      exit(0);
    }
    else {
      wait(&status);
    }

    resetStream(tupleStream);
  }
}

inline bool hasDictionary(char counter) {
  return counter < Dictionaries::Count;
}

inline Dictionary* getDictionary(char counter) {
  switch (counter) {
    case 0:
      return new UncompressedDictionary();
    case 1:
      return new SimpleDictionary();
    case 2:
      return new HashARTDictionary();
    case 3:
      return new ARTDictionary();
    case 4:
      return new BTreeDictionary();
    case 5:
      return new BPlusTreeDictionary();
  }
  assert(false);
  return nullptr;
}

inline uint64_t getNumberOfValues(istream& tupleStream) {
  TurtleParser parser(tupleStream);
  DummyDictionary dict;
  parser.loadInto(&dict);
  resetStream(tupleStream);
  return dict.size();
}

inline void resetStream(istream& tupleStream) {
  tupleStream.clear();
  tupleStream.seekg(0, ios_base::beg);
}

inline uint64_t* getRandomIDs(uint64_t numberOfOperations, uint64_t numberOfValues) {
  std::random_device device;
  std::mt19937_64 engine(device());
  std::uniform_int_distribution<uint64_t> dist(0, numberOfValues-1);

  uint64_t* identifiers = new uint64_t[numberOfOperations];
  for (uint64_t i = 0; i < numberOfOperations; i++) {
    identifiers[i] = dist(engine);
  }
  return identifiers;
}

inline float diff(clock_t start) {
  float diff = static_cast<float>(clock() - start);
  return diff / CLOCKS_PER_SEC;
}
