#include "PerformanceTestRunner.hpp"
#include "TurtleParser.hpp"
#include "Dictionaries.hpp"
#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <cstdlib>

using namespace std;

#define BULK_LOAD_RATIO 0.8

inline bool hasDictionary(char counter);
inline Dictionary* getDictionary(char counter);

inline unordered_set<string> getValues(istream& tupleStream);

/**
 * Creates an array of size numberOfRandomIDs with random values in the range of [lower, upper[.
 */
inline vector<uint64_t> getRandomIDs(uint64_t numberOfRandomIDs, uint64_t lower, uint64_t upper);
inline vector<string> getValues(vector<uint64_t> randomIDs, unordered_set<string> values);
inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, unordered_set<string> values);
inline vector<string> getPrefixes(vector<uint64_t> randomIDs, unordered_set<string> values);
inline void splitForBulkLoad(vector<uint64_t> insertIDs, unordered_set<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues);

inline void bulkLoad(Dictionary*, vector<string>);
inline void insert(Dictionary*, vector<string>);
inline void lookup(Dictionary*, vector<uint64_t>);
inline void lookup(Dictionary*, vector<string>);
inline void rangeLookup(Dictionary*, vector<uint64_t>);
inline void rangeLookup(Dictionary*, vector<string>);
inline void update(Dictionary*, vector<uint64_t>, vector<string>);

inline float diff(clock_t start);
inline uint64_t getMemoryUsage();

void PerformanceTestRunner::run(istream& tupleStream) {
  cout << "Reading string values from turtle file into memory." << endl;
  unordered_set<string> uniqueValues = getValues(tupleStream);
  uint64_t numberOfUniqueValues = uniqueValues.size();
  uint64_t numberOfBulkLoadValues = static_cast<uint64_t>(BULK_LOAD_RATIO * numberOfUniqueValues);
  uint64_t numberOfOperations = numberOfUniqueValues-numberOfBulkLoadValues;

  cout << " " << "Turtle file contains " << numberOfUniqueValues << " unique string values." << endl;

  cout << "Generating random numbers..." << endl;
  vector<uint64_t> insertIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> lookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> rangeLookupIDs = getRandomIDs(2*numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> neLookupIDs = getRandomIDs(numberOfOperations, numberOfUniqueValues, 2*numberOfUniqueValues);
  vector<uint64_t> updateIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);

  vector<string> bulkLoadValues, insertValues;
  bulkLoadValues.reserve(numberOfBulkLoadValues);
  insertValues.reserve(numberOfOperations);
  splitForBulkLoad(insertIDs, uniqueValues, bulkLoadValues, insertValues);

  vector<uint64_t> valueLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> valueRangeLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> neValueLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> valueUpdateIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<string> lookupValues = getValues(valueLookupIDs, uniqueValues);
  vector<string> rangeLookupValues = getPrefixes(valueRangeLookupIDs, uniqueValues);
  vector<string> neLookupValues = getNonExistingValues(neValueLookupIDs, uniqueValues);
  vector<string> updateValues = getValues(valueUpdateIDs, uniqueValues);

  valueRangeLookupIDs.clear();
  neValueLookupIDs.clear();
  valueLookupIDs.clear();
  valueUpdateIDs.clear();
  uniqueValues.clear();

  cout << endl;

  uint64_t baseMemory = getMemoryUsage();

  // Load data from into all dictionaries in succession
  for (char counter = 0; hasDictionary(counter); counter++) {
    Dictionary* dict = getDictionary(counter);

    cout << "# Executing tests with " << dict << "." << endl;

    clock_t start;

    cout << "Bulk-loading " << numberOfBulkLoadValues << " values." << endl;
    start = clock();
    bulkLoad(dict, bulkLoadValues);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Inserting " << numberOfOperations << " values." << endl;
    start = clock();
    insert(dict, insertValues);
    cout << " Finished in " << diff(start) << " sec." << endl;
    cout << "Memory usage: " << getMemoryUsage() - baseMemory << "." << endl;

    cout << "Looking up " << numberOfOperations << " strings by ID." << endl;
    start = clock();
    lookup(dict, lookupIDs);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Looking up " << numberOfOperations << " strings by value." << endl;
    start = clock();
    lookup(dict, lookupValues);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Executing " << numberOfOperations << " range lookups by ID." << endl;
    start = clock();
    rangeLookup(dict, rangeLookupIDs);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Executing " << numberOfOperations << " range lookups by value." << endl;
    start = clock();
    rangeLookup(dict, rangeLookupValues);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Looking up " << numberOfOperations << " non-existing strings by ID." << endl;
    start = clock();
    lookup(dict, neLookupIDs);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Looking up " << numberOfOperations << " non-existing strings by value." << endl;
    start = clock();
    lookup(dict, neLookupValues);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << "Updating " << numberOfOperations << " strings." << endl;
    start = clock();
    update(dict, updateIDs, updateValues);
    cout << " Finished in " << diff(start) << " sec." << endl;

    cout << endl;

    delete dict;
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
      return new ARTcDictionary();
    case 5:
      return new BTreeDictionary();
    case 6:
      return new BPlusTreeDictionary();
  }
  assert(false);
  return nullptr;
}

class PerfTestDictionary : public Dictionary {
  public:
    unordered_set<std::string> values;

    ~PerfTestDictionary() noexcept { }

    uint64_t insert(std::string value);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    bool bulkInsert(size_t size, std::string* v) {
      return false;
    }

    bool update(uint64_t& id, std::string value) {
      return false;
    }

    bool lookup(std::string value, uint64_t& id) {
      return false;
    }

    bool lookup(uint64_t id, std::string& value) {
      return false;
    }
#pragma GCC diagnostic pop

    const char* name() const {
      return "PerformanceTestDictionary";
    }
};

uint64_t PerfTestDictionary::insert(string value) {
  values.insert(value);
  return 0;
}

inline unordered_set<string> getValues(istream& tupleStream) {
  TurtleParser parser(tupleStream);
  PerfTestDictionary dict;
  parser.loadInto(&dict);
  return dict.values;
}

inline vector<uint64_t> getRandomIDs(uint64_t numberOfOperations, uint64_t lower, uint64_t upper) {
  random_device device;
  mt19937_64 engine(device());
  uniform_int_distribution<uint64_t> dist(lower, upper-1);

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

inline void bulkLoad(Dictionary* dict, vector<string> values) {
  dict->bulkInsert(values.size(), &values[0]);
}

inline void insert(Dictionary* dict, vector<string> values) {
  for (auto value : values) {
    dict->insert(value);
  }
}
inline void lookup(Dictionary* dict, vector<uint64_t> ids) {
  for (auto id : ids) {
    string value;
    dict->lookup(id, value);
  }
}

inline void lookup(Dictionary* dict, vector<string> values) {
  for (auto value : values) {
    uint64_t id;
    dict->lookup(value, id);
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void rangeLookup(Dictionary* dict, vector<uint64_t> ids) {
  for (size_t i = 0; i < ids.size(); i += 2) {
    uint64_t from = ids[i];
    uint64_t to = ids[i+1];
    // Swap into correct order
    if (from > to) {
      uint64_t tmp = from;
      from = to;
      to = tmp;
    }
    //TODO: range lookup
    // dict->rangeLookup(from, to);
  }
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
inline void rangeLookup(Dictionary* dict, vector<string> prefixes) {
  for (string prefix : prefixes) {
    //TODO: range lookup
    // dict->rangeLookup(prefix);
  }
}
#pragma GCC diagnostic pop

inline void update(Dictionary* dict, vector<uint64_t> ids, vector<string> values) {
  for (size_t i = 0; i < ids.size(); i++) {
    dict->update(ids[i], values[i]);
  }
}

inline void splitForBulkLoad(vector<uint64_t> insertIDs, unordered_set<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  unordered_set<uint64_t> idSet(insertIDs.begin(), insertIDs.end());
  for (size_t i = 0; i < values.size(); i++) {
    auto it = idSet.find(i);
    if (it != idSet.end()) {
      insertValues.push_back(valueVector[i]);
    }
    else {
      bulkLoadValues.push_back(valueVector[i]);
    }
  }
}

inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, unordered_set<string> values) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  vector<string> result;
  result.reserve(randomIDs.size());
  for (uint64_t id : randomIDs) {
    result.push_back(valueVector[id] + static_cast<char>(254));
  }
  return result;
}

inline vector<string> getValues(vector<uint64_t> randomIDs, unordered_set<string> values) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  vector<string> result;
  result.reserve(randomIDs.size());
  for (uint64_t id : randomIDs) {
    result.push_back(valueVector[id]);
  }
  return result;
}

inline uint64_t getMemoryUsage() {
  //TODO: /proc/*/smaps more precise?
  fstream smaps("/proc/self/status", ios_base::in);
  string line;
  uint64_t memory = 0;
  while (getline(smaps, line).good()) {
    if (boost::starts_with(line, "VmRSS")) {
      assert(memory == 0);
      memory = static_cast<uint64_t>(atol(line.c_str() + 6));
    }
  }
  smaps.close();
  return memory;
}

inline vector<string> getPrefixes(vector<uint64_t> randomIDs, unordered_set<string> values) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  vector<string> result;
  result.reserve(randomIDs.size());

  random_device device;
  mt19937_64 engine(device());

  for (uint64_t id : randomIDs) {
    uniform_int_distribution<uint64_t> dist(3, valueVector[id].size());
    result.push_back(valueVector[id].substr(0, dist(engine)));
  }
  return result;
}
