#include "MARTDictionary.hpp"
#include <cassert>

using namespace std;

uint64_t MARTDictionary::insert(string value) {
  uint64_t id;
  if (!reverseIndex.lookup(value, id)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(nextId, value);
    return nextId++;
  }
  return id;
}

bool MARTDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  reverseIndex.bulkInsert(size, values);

  for (; nextId < size; nextId++) {
    index.insert(nextId, values[nextId]);
  }

  return true;
}

bool MARTDictionary::update(uint64_t& id, std::string value) {
  std::string storedValue;
  if (!index.lookup(id, storedValue)) {
    return false;
  }

  if (value != storedValue) {
    id = insert(value);
  }
  return true;
}

bool MARTDictionary::lookup(std::string value, uint64_t& id) {
  return reverseIndex.lookup(value, id);
}

bool MARTDictionary::lookup(uint64_t id, std::string& value) {
  return index.lookup(id, value);
}
