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

      cout << "Inserting " << numberOfValues << " strings into " << dict << "." << endl;
      start = clock();
      TurtleParser parser(tupleStream);
      parser.loadInto(dict);
      cout << "Finished in " << diff(start) << " sec." << endl;

      cout << "Memory usage:" << endl;
      system(("ps -p " + to_string(getpid()) + " -orss=").c_str());

      uint64_t* identifiers = getRandomIDs(numberOfOperations, dict->size());

      cout << "Looking up " << numberOfOperations << " strings by ID." << endl;
      string* values = new string[numberOfOperations];
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
        dict->lookup(identifiers[i], values[i]);
      }
      cout << "Finished in " << diff(start) << " sec." << endl;

      cout << "Looking up " << numberOfOperations << " strings by value." << endl;
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
        uint64_t id;
        dict->lookup(values[i], id);
      }
      cout << "Finished in " << diff(start) << " sec." << endl;

      cout << "Updating " << numberOfOperations << " strings." << endl;
      start = clock();
      for (uint64_t i = 0; i < numberOfOperations; i++) {
        dict->update(identifiers[i], values[numberOfOperations-1-i]);
      }
      cout << "Finished in " << diff(start) << " sec." << endl;

      delete[] values;
      delete[] identifiers;
      delete dict;
      cout << "---" << endl;
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
  random_device device;
  mt19937_64 engine(device());
  uniform_int_distribution<uint64_t> dist(0, numberOfValues-1);

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
