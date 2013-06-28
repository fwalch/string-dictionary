#include "ARTDictionary.hpp"

using namespace std;

uint64_t ARTDictionary::insert(string value) {
  uint64_t id;
  if (!reverseIndex.lookup(value, id)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(nextId, value);
    return nextId++;
  }
  return id;
}

bool ARTDictionary::bulkInsert(size_t size, string* values) {
  for (size_t i = 0; i < size; i++) {
    insert(values[i]);
  }

  return true;
}

bool ARTDictionary::update(uint64_t& id, std::string value) {
  std::string storedValue;
  if (!index.lookup(id, storedValue)) {
    return false;
  }

  if (value != storedValue) {
    id = insert(value);
  }
  return true;
}

bool ARTDictionary::lookup(std::string value, uint64_t& id) {
  return reverseIndex.lookup(value, id);
}

bool ARTDictionary::lookup(uint64_t id, std::string& value) {
  return index.lookup(id, value);
}
