#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>
#include <string>
#include <ctime>
#include <sys/wait.h>
#include <set>
#include <vector>
#include "PerformanceTestRunner.hpp"
#include "ArtLeafRunner.hpp"
#include "rdf3x/TurtleParser.hpp"

#include "StringDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "Indexes.hpp"
#include "Pages.hpp"

using namespace std;

#define BULK_LOAD_RATIO 1.0

inline bool hasDictionary(char counter);
inline Dictionary* getDictionary(char counter);

inline vector<string> getValues(istream& tupleStream);

/**
 * Creates an array of size numberOfRandomIDs with random values in the range of [lower, upper[.
 */
inline vector<uint64_t> getRandomIDs(uint64_t numberOfRandomIDs, uint64_t lower, uint64_t upper);
inline vector<string> getValues(vector<uint64_t> randomIDs, vector<string> values);
inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, vector<string> values);
inline vector<string> getPrefixes(vector<uint64_t> randomIDs, vector<string> values, uint64_t globalPrefixLength);
inline void splitForBulkLoad(vector<uint64_t> insertIDs, vector<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues);
inline uint64_t getGlobalPrefixLength(vector<string>::const_iterator, vector<string>::const_reverse_iterator);

inline void bulkLoad(Dictionary*, vector<string>&);
inline void insert(Dictionary*, vector<string>&);
template<bool check>
inline void lookup(Dictionary*, vector<uint64_t>&);
template<bool check>
inline void lookup(Dictionary*, vector<string>&);
inline void rangeLookup(Dictionary*, vector<string>&, vector<pair<uint64_t, string>>&, bool check);

inline float diff(clock_t start);
inline uint64_t getMemoryUsage();

class MicroTestLeafStore : public LeafStore {
  private:
    vector<string>& values;
  public:
    MicroTestLeafStore(vector<string>& values) : values(values) { }
    std::string getValue(uint64_t leafValue) const;
    uint64_t getId(uint64_t leafValue) const;
};

std::string MicroTestLeafStore::getValue(uint64_t leafValue) const {
  return values[leafValue-1];
}

uint64_t MicroTestLeafStore::getId(uint64_t leafValue) const {
  return leafValue;
}

template<template<typename> class TIndex, uint64_t numberOfOperations>
void runMicroStringTest(MicroTestLeafStore& leafStore, vector<string>& uniqueValues, vector<string>& lookupValues) {
  uint64_t numberOfUniqueValues = uniqueValues.size();
  int status;
  if (fork() == 0) {
    TIndex<string> reverseIndex = ConstructHelper<TIndex<string>>::create(&leafStore);
    uint64_t baseMemory = getMemoryUsage();
    clock_t start;
    float df;

    cout << reverseIndex.description() << "\t";

    start = clock();
    for (uint64_t i = 0; i < numberOfUniqueValues; i++) {
      reverseIndex.insert(uniqueValues[i], i);
    }
    df = diff(start);
    cout << numberOfUniqueValues/df << "\t";

    cout << getMemoryUsage()-baseMemory << "\t";

    start = clock();
      for (auto value : lookupValues) {
        uint64_t leafValue;
#ifdef DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
        bool result = reverseIndex.lookup(value, leafValue);
        assert(result);
        assert(uniqueValues[leafValue] == value);
#pragma GCC diagnostic pop
#else
        reverseIndex.lookup(value, leafValue);
#endif
      }
    df = diff(start);
    cout << numberOfOperations/df << endl;
    exit(0);
  }
  else {
    wait(&status);
  }
}

template<template<typename> class TIndex, uint64_t numberOfOperations>
void runMicroIdTest(MicroTestLeafStore& leafStore, uint64_t numberOfUniqueValues, vector<uint64_t>& lookupIDs) {
  int status;
  if (fork() == 0) {
    TIndex<uint64_t> index = ConstructHelper<TIndex<uint64_t>>::create(&leafStore);
    uint64_t baseMemory = getMemoryUsage();
    clock_t start;
    float df;

    cout << index.description() << "\t";

    start = clock();
    for (uint64_t i = 0; i < numberOfUniqueValues; i++) {
      index.insert(i, i);
    }
    df = diff(start);
    cout << numberOfUniqueValues/df << "\t";

    cout << getMemoryUsage()-baseMemory << "\t";

    start = clock();
      for (auto id : lookupIDs) {
        uint64_t leafValue;
#ifdef DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
        bool result = index.lookup(id, leafValue);
        assert(result);
        assert(leafValue == id);
#pragma GCC diagnostic pop
#else
        index.lookup(id, leafValue);
#endif
      }
    df = diff(start);
    cout << numberOfOperations/df << endl;
    exit(0);
  }
  else {
    wait(&status);
  }
}

/**
 * Executes microbenchmarks for index implementations.
 */
void MicroTestRunner::run(istream& tupleStream) {
  vector<string> uniqueValues = getValues(tupleStream);
  uint64_t numberOfUniqueValues = uniqueValues.size();
  const uint64_t numberOfOperations = 1E7;

  vector<uint64_t> lookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  vector<string> lookupValues = getValues(lookupIDs, uniqueValues);

  MicroTestLeafStore leafStore(uniqueValues);

  std::cout << "ID:" << std::endl;
  std::cout << "name\tbulk_load\tmemory\tlookup" << std::endl;

  // ID tests
  runMicroIdTest<BTree, numberOfOperations>(leafStore, numberOfUniqueValues, lookupIDs);
  runMicroIdTest<BPlusTree, numberOfOperations>(leafStore, numberOfUniqueValues, lookupIDs);
  runMicroIdTest<ART, numberOfOperations>(leafStore, numberOfUniqueValues, lookupIDs);
  runMicroIdTest<RedBlack, numberOfOperations>(leafStore, numberOfUniqueValues, lookupIDs);
  runMicroIdTest<Hash, numberOfOperations>(leafStore, numberOfUniqueValues, lookupIDs);

  std::cout << "String:" << std::endl;
  std::cout << "name\tbulk_load\tmemory\tlookup" << std::endl;
  // String tests
  runMicroStringTest<ART, numberOfOperations>(leafStore, uniqueValues, lookupValues);
  runMicroStringTest<HAT, numberOfOperations>(leafStore, uniqueValues, lookupValues);
  runMicroStringTest<BTree, numberOfOperations>(leafStore, uniqueValues, lookupValues);
  runMicroStringTest<BPlusTree, numberOfOperations>(leafStore, uniqueValues, lookupValues);
  runMicroStringTest<RedBlack, numberOfOperations>(leafStore, uniqueValues, lookupValues);
  runMicroStringTest<Hash, numberOfOperations>(leafStore, uniqueValues, lookupValues);
}

class ArtLeafStore : public LeafStore {
  public:
    std::string getValue(uint64_t leafValue) const;
    uint64_t getId(uint64_t leafValue) const;
};

std::string ArtLeafStore::getValue(uint64_t leafValue) const {
  SingleUncompressedPage<1024*4>* leaf = reinterpret_cast<SingleUncompressedPage<1024*4>*>(leafValue >> 16);
  uint16_t offset = leafValue & 0xFFFF;

  return leaf->getByOffset(offset).getValue();
}

uint64_t ArtLeafStore::getId(uint64_t leafValue) const {
  SingleUncompressedPage<1024*4>* leaf = reinterpret_cast<SingleUncompressedPage<1024*4>*>(leafValue >> 16);
  uint16_t offset = leafValue & 0xFFFF;

  return leaf->getByOffset(offset).getId();
}

void ArtLeafRunner::run(istream& tupleStream) {
  vector<string> uniqueValues = getValues(tupleStream);
  uint64_t numberOfUniqueValues = uniqueValues.size();
  uint64_t numberOfBulkLoadValues = static_cast<uint64_t>(BULK_LOAD_RATIO * numberOfUniqueValues);
  uint64_t numberOfInserts = numberOfUniqueValues-numberOfBulkLoadValues;
  uint64_t numberOfOperations = 1E7;

  vector<uint64_t> insertIDs = getRandomIDs(numberOfInserts, 1, numberOfUniqueValues);
  vector<uint64_t> lookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  //vector<uint64_t> neLookupIDs = getRandomIDs(numberOfOperations, numberOfUniqueValues, 2*numberOfUniqueValues);

  vector<string> bulkLoadValues, insertValues;
  bulkLoadValues.reserve(numberOfBulkLoadValues);
  insertValues.reserve(numberOfInserts);
  splitForBulkLoad(insertIDs, uniqueValues, bulkLoadValues, insertValues);

  uint64_t globalPrefixLength = getGlobalPrefixLength(uniqueValues.cbegin(), uniqueValues.crbegin());

  vector<uint64_t> valueLookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  vector<uint64_t> valueRangeLookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  //vector<uint64_t> neValueLookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  vector<string> lookupValues = getValues(valueLookupIDs, uniqueValues);
  vector<string> rangeLookupValues = getPrefixes(valueRangeLookupIDs, uniqueValues, globalPrefixLength);
  //vector<string> neLookupValues = getNonExistingValues(neValueLookupIDs, uniqueValues);

  valueRangeLookupIDs.clear();
  //neValueLookupIDs.clear();
  valueLookupIDs.clear();
  uniqueValues.clear();

  vector<pair<uint64_t, string>> lookedUpPairs;
  lookedUpPairs.reserve(numberOfOperations);

  std::cout << "name\tbulk_load\tmemory\tnumber_of_pages\tlookup_id\tlookup_string" << std::endl;

  // Fork to allow memory measurements
  int status;
  if (fork() == 0) {
    uint64_t baseMemory = getMemoryUsage();

    ArtLeafStore artLeafStore;
    ART<uint64_t> index(&artLeafStore);
    HAT<std::string> reverseIndex;

    cout << "Independent" << "\t";

    clock_t start;
    float df;

    start = clock();
    uint64_t nextId = 1;
    std::vector<std::pair<uint64_t, std::string>> pairs;
    pairs.reserve(numberOfBulkLoadValues);

    for (size_t i = 0; i <numberOfBulkLoadValues; i++) {
      pairs.push_back(make_pair(nextId++, bulkLoadValues[i]));
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    SingleUncompressedPage<1024*4>::load(pairs, [&index](SingleUncompressedPage<1024*4>* page, uint16_t deltaValue, uint16_t offset, uint64_t id, std::string value) {
        uint64_t leafValue = reinterpret_cast<uint64_t>(page);
        leafValue = leafValue <<16;
        leafValue |= static_cast<uint64_t>(offset);
        index.insert(id, leafValue);
        });
    SingleUncompressedPage<1024*4>::load(pairs, [&reverseIndex](SingleUncompressedPage<1024*4>* page, uint16_t deltaValue, uint16_t offset, uint64_t id, std::string value) {
        uint64_t leafValue = reinterpret_cast<uint64_t>(page);
        leafValue = leafValue <<16;
        leafValue |= static_cast<uint64_t>(offset);
        reverseIndex.insert(value, leafValue);
        });
#pragma GCC diagnostic pop

    df = diff(start);
    cout << numberOfBulkLoadValues/df << "\t";

    /*cout << "  [SKIPPED] Inserting " << numberOfInserts << " values." << endl;
      start = clock();
      insert(dict, insertValues);
      df = diff(start);
      cout << " Finished in " << diff(start) << " sec ("<< numberOfInserts/df <<" tpm)." << endl;*/
    cout << getMemoryUsage() - baseMemory << "\t";

    cout << SingleUncompressedPage<1024*4>::counter << "\t";

    start = clock();
    for (auto id : lookupIDs) {
      uintptr_t value;
      if (index.lookup(id, value)) {
        lookedUpPairs.push_back(make_pair(id, artLeafStore.getValue(value)));
      }
    }
    df = diff(start);
    cout << numberOfOperations/df << "\t";
    lookedUpPairs.clear();

    start = clock();
    for (string& str : lookupValues) {
      uint64_t value;
      if (reverseIndex.lookup(str, value)) {
        lookedUpPairs.push_back(make_pair(artLeafStore.getId(value), str));
      }
    }
    df = diff(start);
    cout << numberOfOperations/df << "\t";

    /*start = clock();
      rangeLookup(dict, rangeLookupValues, lookedUpPairs, true);
      df = diff(start);
      cout << numberOfOperations/df << "\t";
      lookedUpPairs.clear();*/

    /*start = clock();
      lookup(dict, neLookupIDs, lookedUpPairs, false);
      df = diff(start);
      cout << numberOfOperations/df << "\t";

      start = clock();
      lookup(dict, neLookupValues, lookedUpPairs, false);
      df = diff(start);
      cout << numberOfOperations/df << "\t";*/

    cout << endl;

    exit(0);
  }
  else {
    wait(&status);
  }
}

/**
 * Executes performance tests for dictionary implementations.
 */
void PerformanceTestRunner::run(istream& tupleStream) {
  vector<string> uniqueValues = getValues(tupleStream);
  uint64_t numberOfUniqueValues = uniqueValues.size();
  uint64_t numberOfBulkLoadValues = static_cast<uint64_t>(BULK_LOAD_RATIO * numberOfUniqueValues);
  uint64_t numberOfInserts = numberOfUniqueValues-numberOfBulkLoadValues;
  uint64_t numberOfOperations = 1E7;

  vector<uint64_t> insertIDs = getRandomIDs(numberOfInserts, 1, numberOfUniqueValues);
  vector<uint64_t> lookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);

  vector<string> bulkLoadValues, insertValues;
  bulkLoadValues.reserve(numberOfBulkLoadValues);
  insertValues.reserve(numberOfInserts);
  splitForBulkLoad(insertIDs, uniqueValues, bulkLoadValues, insertValues);

  vector<uint64_t> valueLookupIDs = getRandomIDs(numberOfOperations, 1, numberOfUniqueValues);
  vector<string> lookupValues = getValues(valueLookupIDs, uniqueValues);
  valueLookupIDs.clear();
  uniqueValues.clear();

  std::cout << "name\tbulk_load\tmemory\tnumber_of_pages\tlookup_id\tlookup_string" << std::endl;

  // Load data from into all dictionaries in succession
  for (char counter = 0; hasDictionary(counter); counter++) {
    // Fork to allow memory measurements
    int status;
    if (fork() == 0) {
      uint64_t baseMemory = getMemoryUsage();

      Dictionary* dict = getDictionary(counter);

      cout << dict << "\t";

      clock_t start;
      float df;

      start = clock();
      bulkLoad(dict, bulkLoadValues);
      df = diff(start);
      cout << numberOfBulkLoadValues/df << "\t";

      cout << getMemoryUsage() - baseMemory << "\t";

      cout << dict->numberOfLeaves() << "\t";

      start = clock();
      lookup<true>(dict, lookupIDs);
      df = diff(start);
      cout << numberOfOperations/df << "\t";

      start = clock();
      lookup<true>(dict, lookupValues);
      df = diff(start);
      cout << numberOfOperations/df << "\t";

      cout << endl;

      delete dict;

      exit(0);
    }
    else {
      wait(&status);
    }
  }
}

inline bool hasDictionary(char counter) {
  return counter < 7;
}

inline Dictionary* getDictionary(char counter) {
  switch (counter) {
    case 0:
      return new StringDictionary<ART, SART, BottomUpPage<(1024<<0)>, BottomUpStrategy>();
    case 1:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<1)>>();
    case 2:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<2)>>();
    case 3:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<3)>>();
    case 4:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<4)>>();
    case 5:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<5)>>();
    case 6:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<6)>>();
  }
  throw;
}

inline vector<string> getValues(istream& tupleStream) {
  TurtleParser parser(tupleStream);
  set<string> values;

  while (true) {
    string subject,predicate,object,objectSubType;
    Type::ID objectType;
    try {
      if (!parser.parse(subject,predicate,object,objectType,objectSubType)) {
        break;
      }
    }
    catch (const TurtleParser::Exception& e) {
      cerr << e.message << endl;
      // recover...
      while (tupleStream.get()!='\n') ;
      continue;
    }

    values.insert(subject);
    values.insert(predicate);
    values.insert(object);
  }

  return vector<string>(values.begin(), values.end());
}

inline vector<uint64_t> getRandomIDs(uint64_t numberOfOperations, uint64_t lower, uint64_t upper) {
  random_device device;
  mt19937_64 engine(device());
  uniform_int_distribution<uint64_t> dist(lower, upper);

  vector<uint64_t> identifiers;
  identifiers.reserve(numberOfOperations);
  while (identifiers.size() != numberOfOperations) {
    identifiers.push_back(dist(engine));
  }
  return identifiers;
}

inline float diff(clock_t start) {
  float diff = static_cast<float>(clock() - start);
  return diff / CLOCKS_PER_SEC;
}

inline void bulkLoad(Dictionary* dict, vector<string>& values) {
  dict->bulkInsert(values.size(), &values[0]);
}

inline void insert(Dictionary* dict, vector<string>& values) {
  for (auto value : values) {
    dict->insert(value);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<bool check>
inline void lookup(Dictionary* dict, vector<uint64_t>& ids) {
  for (auto id : ids) {
    string value;
    bool result = dict->lookup(id, value);
    if (!result && check) {
      throw Exception("Entry with ID " + std::to_string(id) + "not found.");
    }
  }
  /*
#ifdef DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
if (check) {
// Verify
for (auto pair : lookedUpPairs) {
uint64_t id;
if (!dict->lookup(pair.second, id)) {
std::cout << "Entry not found for " << pair.second << "; debug." << std::endl;
dict->debug();
std::cout << "# second lookup" << std::endl;
dict->lookup(pair.second, id);
throw Exception("Entry not found; debug.");
}
assert(id == pair.first);
}
}
#pragma GCC diagnostic pop
#endif
*/
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<bool check>
inline void lookup(Dictionary* dict, vector<string>& values) {
  for (string& value : values) {
    uint64_t id;
    bool result = dict->lookup(value, id);
    if (!result && check) {
      throw Exception("Entry with value "+ value +" not found; debug.");
    }
  }
  /*
#ifdef DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
if (check) {
// Verify
for (auto pair : lookedUpPairs) {
uint64_t id;
assert(dict->lookup(pair.second, id));
assert(id == pair.first);
}
}
#pragma GCC diagnostic pop
#endif
*/
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void rangeLookup(Dictionary* dict, vector<string>& prefixes, vector<pair<uint64_t, string>>& lookedUpPairs, bool check) {
  for (string prefix : prefixes) {
    dict->rangeLookup(prefix, [&](uint64_t id, std::string value) {
        lookedUpPairs.push_back(make_pair(id, value));
        });
#ifdef DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    if (check) {
      // Verify
      for (auto pair : lookedUpPairs) {
        string value;
        assert(dict->lookup(pair.first, value));
        assert(value == pair.second);
        assert(boost::starts_with(value, prefix));
      }
    }
#pragma GCC diagnostic pop
#endif
    lookedUpPairs.clear();
  }
}
#pragma GCC diagnostic pop

inline void splitForBulkLoad(vector<uint64_t> insertIDs, vector<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues) {
  set<uint64_t> idSet(insertIDs.begin(), insertIDs.end());
  for (size_t i = 0; i < values.size(); i++) {
    auto it = idSet.find(i);
    if (it != idSet.end()) {
      insertValues.push_back(values[i]);
    }
    else {
      bulkLoadValues.push_back(values[i]);
    }
  }

  sort(bulkLoadValues.begin(), bulkLoadValues.end());
}

inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, vector<string> values) {
  vector<string> result;
  result.reserve(randomIDs.size());
  for (uint64_t id : randomIDs) {
    result.push_back(values[id-1] + static_cast<char>(254));
  }
  return result;
}

inline vector<string> getValues(vector<uint64_t> randomIDs, vector<string> values) {
  vector<string> result;
  result.reserve(randomIDs.size());
  for (uint64_t id : randomIDs) {
    result.push_back(values[id-1]);
  }
  return result;
}

inline uint64_t getMemoryUsage() {
  //TODO: /proc/*/smaps more precise?
  FILE* proc = popen(("ps -p"+ to_string(getpid()) +" -orss=").c_str(), "r");
  assert(proc);
  char buffer[64];
  string result = "";
  while (!feof(proc)) {
    if (fgets(buffer, 64, proc) != NULL) {
      result += buffer;
    }
  }
  pclose(proc);
  return static_cast<uint64_t>(atol(result.c_str()));
  /*fstream status("/proc/self/status", ios_base::in);
    string line;
    uint64_t memory = 0;
    while (getline(status, line).good()) {
    if (boost::starts_with(line, "VmRSS")) {
    assert(memory == 0);
    memory = static_cast<uint64_t>(atol(line.c_str() + 6));
    }
    }
    status.close();
    return memory;*/
}

inline vector<string> getPrefixes(vector<uint64_t> randomIDs, vector<string> values, uint64_t globalPrefixLength) {
  vector<string> result;
  result.reserve(randomIDs.size());

  random_device device;
  mt19937_64 engine(device());

  for (uint64_t id : randomIDs) {
    uniform_int_distribution<uint64_t> dist(globalPrefixLength, values[id-1].size());
    result.push_back(values[id-1].substr(0, dist(engine)));
  }
  return result;
}

inline uint64_t getGlobalPrefixLength(vector<string>::const_iterator start, vector<string>::const_reverse_iterator end) {
  string first = *start;
  string last = *end;

  uint64_t pos = 0;
  while (first[pos] == last[pos]) {
    pos++;
  }

  return pos;
}
