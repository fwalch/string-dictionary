#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <set>
#include <vector>
#include "PerformanceTestRunner.hpp"
#include "TurtleParser.hpp"

#include "StringDictionary.hpp"
#include "Indexes.hpp"
#include "Pages.hpp"

using namespace std;

#define BULK_LOAD_RATIO 1.0
#define OP_RATIO 0.3

inline bool hasDictionary(char counter);
inline Dictionary* getDictionary(char counter);

inline set<string> getValues(istream& tupleStream);

/**
 * Creates an array of size numberOfRandomIDs with random values in the range of [lower, upper[.
 */
inline vector<uint64_t> getRandomIDs(uint64_t numberOfRandomIDs, uint64_t lower, uint64_t upper);
inline vector<string> getValues(vector<uint64_t> randomIDs, set<string> values);
inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, set<string> values);
inline vector<string> getPrefixes(vector<uint64_t> randomIDs, set<string> values, uint64_t globalPrefixLength);
inline void splitForBulkLoad(vector<uint64_t> insertIDs, set<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues);
inline uint64_t getGlobalPrefixLength(set<string>::const_iterator, set<string>::const_reverse_iterator);

inline void bulkLoad(Dictionary*, vector<string>);
inline void insert(Dictionary*, vector<string>);
inline void lookup(Dictionary*, vector<uint64_t>, vector<pair<uint64_t, string>>);
inline void lookup(Dictionary*, vector<string>, vector<pair<uint64_t, string>>);
inline void rangeLookup(Dictionary*, vector<string>, vector<pair<uint64_t, string>>);

inline float diff(clock_t start);
inline uint64_t getMemoryUsage();

/**
 * Executes performance tests for dictionary implementations.
 */
void PerformanceTestRunner::run(istream& tupleStream) {
  cout << "Reading string values from turtle file into memory." << endl;
  set<string> uniqueValues = getValues(tupleStream);
  uint64_t numberOfUniqueValues = uniqueValues.size();
  uint64_t numberOfBulkLoadValues = static_cast<uint64_t>(BULK_LOAD_RATIO * numberOfUniqueValues);
  uint64_t numberOfInserts = numberOfUniqueValues-numberOfBulkLoadValues;
  uint64_t numberOfOperations = static_cast<uint64_t>(OP_RATIO * numberOfUniqueValues);

  cout << " " << "Turtle file contains " << numberOfUniqueValues << " unique string values." << endl;

  cout << "Generating random numbers..." << endl;
  vector<uint64_t> insertIDs = getRandomIDs(numberOfInserts, 0, numberOfUniqueValues);
  vector<uint64_t> lookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> neLookupIDs = getRandomIDs(numberOfOperations, numberOfUniqueValues, 2*numberOfUniqueValues);

  vector<string> bulkLoadValues, insertValues;
  bulkLoadValues.reserve(numberOfBulkLoadValues);
  insertValues.reserve(numberOfInserts);
  splitForBulkLoad(insertIDs, uniqueValues, bulkLoadValues, insertValues);

  uint64_t globalPrefixLength = getGlobalPrefixLength(uniqueValues.cbegin(), uniqueValues.crbegin());

  vector<uint64_t> valueLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> valueRangeLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<uint64_t> neValueLookupIDs = getRandomIDs(numberOfOperations, 0, numberOfUniqueValues);
  vector<string> lookupValues = getValues(valueLookupIDs, uniqueValues);
  vector<string> rangeLookupValues = getPrefixes(valueRangeLookupIDs, uniqueValues, globalPrefixLength);
  vector<string> neLookupValues = getNonExistingValues(neValueLookupIDs, uniqueValues);

  valueRangeLookupIDs.clear();
  neValueLookupIDs.clear();
  valueLookupIDs.clear();
  uniqueValues.clear();

  vector<pair<uint64_t, string>> lookedUpPairs;
  lookedUpPairs.reserve(numberOfOperations);

  // Load data from into all dictionaries in succession
  for (char counter = 0; hasDictionary(counter); counter++) {
    // Fork to allow memory measurements
    int status;
    if (fork() == 0) {
      uint64_t baseMemory = getMemoryUsage();

      Dictionary* dict = getDictionary(counter);

      cout << endl;
      cout << "> Executing tests with " << dict << "." << endl;

      clock_t start;

      cout << "  Bulk-loading " << numberOfBulkLoadValues << " values." << endl;
      start = clock();
      bulkLoad(dict, bulkLoadValues);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "  [SKIPPED] Inserting " << numberOfInserts << " values." << endl;
      /*start = clock();
      insert(dict, insertValues);
      cout << " Finished in " << diff(start) << " sec." << endl;*/
      cout << "  Memory usage: " << getMemoryUsage() - baseMemory << "." << endl;

      cout << "  Looking up " << numberOfOperations << " strings by ID." << endl;
      start = clock();
      lookup(dict, lookupIDs, lookedUpPairs);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "  Looking up " << numberOfOperations << " strings by value." << endl;
      start = clock();
      lookup(dict, lookupValues, lookedUpPairs);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "  Executing " << numberOfOperations << " range lookups by value." << endl;
      start = clock();
      rangeLookup(dict, rangeLookupValues, lookedUpPairs);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "  Looking up " << numberOfOperations << " non-existing strings by ID." << endl;
      start = clock();
      lookup(dict, neLookupIDs, lookedUpPairs);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "  Looking up " << numberOfOperations << " non-existing strings by value." << endl;
      start = clock();
      lookup(dict, neLookupValues, lookedUpPairs);
      cout << "    Finished in " << diff(start) << " sec." << endl;

      cout << "Finished run for " << dict << "." << endl;

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
//    case 0:
//      return new StringDictionary<ART, HAT, MultiUncompressedPage<(1024>>1), 8>>();
//    case 1:
//      return new StringDictionary<ART, HAT, MultiUncompressedPage<(1024>>0), 8>>();
//    case 2:
//      return new StringDictionary<ART, HAT, MultiUncompressedPage<(1024<<1), 8>>();
//    case 3:
//      return new StringDictionary<ART, HAT, MultiUncompressedPage<(1024<<2), 8>>();
//    case 4:
//      return new StringDictionary<ART, HAT, MultiUncompressedPage<(1024<<3), 8>>();
//    case 5:
//      return new StringDictionary<ART, ART, MultiUncompressedPage<(1024<<4), 8>>();
    case 0:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024>>2)>>();
    case 1:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024>>1)>>();
    case 2:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<0)>>();
    case 3:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<1)>>();
    case 4:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<2)>>();
    case 5:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<3)>>();
    case 6:
      return new StringDictionary<ART, HAT, SingleUncompressedPage<(1024<<4)>>();
//    case 0:
//      return new StringDictionary<ART, HAT, DynamicPage<1>>();
//    case 1:
//      return new StringDictionary<ART, HAT, DynamicPage<24>>();
//    case 2:
//      return new StringDictionary<ART, HAT, DynamicPage<28>>();
//    case 3:
//      return new StringDictionary<ART, HAT, DynamicPage<32>>();
//    case 4:
//      return new StringDictionary<ART, HAT, DynamicPage<36>>();
//    case 5:
//      return new StringDictionary<ART, HAT, DynamicPage<40>>();
//    case 6:
//      return new StringDictionary<ART, HAT, DynamicPage<44>>();
//    case 7:
//      return new StringDictionary<ART, HAT, DynamicPage<48>>();
//    case 8:
//      return new StringDictionary<ART, HAT, DynamicPage<52>>();
//    case 9:
//      return new StringDictionary<ART, HAT, DynamicPage<56>>();
//    case 10:
//      return new StringDictionary<ART, HAT, DynamicPage<60>>();
//    case 11:
//      return new StringDictionary<ART, HAT, DynamicPage<64>>();
  }
  throw;
}

class PerfTestDictionary : public Dictionary {
  public:
    set<std::string> values;

    ~PerfTestDictionary() noexcept { }

    uint64_t insert(std::string value);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    void bulkInsert(size_t size, std::string* v) {
      throw;
    }

    bool lookup(std::string& value, uint64_t& id) const {
      throw;
    }

    bool lookup(uint64_t id, std::string& value) const {
      throw;
    }

    void rangeLookup(std::string& prefix, RangeLookupCallbackType callback) const {
      throw;
    }
#pragma GCC diagnostic pop

    std::string description() const {
      return "PerformanceTestDictionary";
    }
};

uint64_t PerfTestDictionary::insert(string value) {
  values.insert(value);
  return 0;
}

inline set<string> getValues(istream& tupleStream) {
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

inline void lookup(Dictionary* dict, vector<uint64_t> ids, vector<pair<uint64_t, string>> lookedUpPairs) {
  for (auto id : ids) {
    string value;
    if (dict->lookup(id, value)) {
      lookedUpPairs.push_back(make_pair(id, value));
    }
  }
  lookedUpPairs.clear();
}

inline void lookup(Dictionary* dict, vector<string> values, vector<pair<uint64_t, string>> lookedUpPairs) {
  for (auto value : values) {
    uint64_t id;
    if (dict->lookup(value, id)) {
      lookedUpPairs.push_back(make_pair(id, value));
    }
  }
  lookedUpPairs.clear();
}

inline void rangeLookup(Dictionary* dict, vector<string> prefixes, vector<pair<uint64_t, string>> lookedUpPairs) {
  for (string prefix : prefixes) {
    dict->rangeLookup(prefix, [&](uint64_t id, std::string value) {
      lookedUpPairs.push_back(make_pair(id, value));
    });
    lookedUpPairs.clear();
  }
}

inline void splitForBulkLoad(vector<uint64_t> insertIDs, set<string> values, vector<string>& bulkLoadValues, vector<string>& insertValues) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  set<uint64_t> idSet(insertIDs.begin(), insertIDs.end());
  for (size_t i = 0; i < values.size(); i++) {
    auto it = idSet.find(i);
    if (it != idSet.end()) {
      insertValues.push_back(valueVector[i]);
    }
    else {
      bulkLoadValues.push_back(valueVector[i]);
    }
  }

  sort(bulkLoadValues.begin(), bulkLoadValues.end());
}

inline vector<string> getNonExistingValues(vector<uint64_t> randomIDs, set<string> values) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  vector<string> result;
  result.reserve(randomIDs.size());
  for (uint64_t id : randomIDs) {
    result.push_back(valueVector[id] + static_cast<char>(254));
  }
  return result;
}

inline vector<string> getValues(vector<uint64_t> randomIDs, set<string> values) {
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

inline vector<string> getPrefixes(vector<uint64_t> randomIDs, set<string> values, uint64_t globalPrefixLength) {
  //TODO
  vector<string> valueVector(values.begin(), values.end());
  vector<string> result;
  result.reserve(randomIDs.size());

  random_device device;
  mt19937_64 engine(device());

  for (uint64_t id : randomIDs) {
    uniform_int_distribution<uint64_t> dist(globalPrefixLength, valueVector[id].size());
    result.push_back(valueVector[id].substr(0, dist(engine)));
  }
  return result;
}

inline uint64_t getGlobalPrefixLength(set<string>::const_iterator start, set<string>::const_reverse_iterator end) {
  string first = *start;
  string last = *end;

  uint64_t pos = 0;
  while (first[pos] == last[pos]) {
    pos++;
  }

  return pos;
}
