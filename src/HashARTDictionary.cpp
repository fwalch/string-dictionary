#include "HashARTDictionary.hpp"

using namespace std;

uint64_t HashARTDictionary::insert(string value) {
  uint64_t id;
  if (!reverseIndex.lookup(value, id)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(make_pair(nextId, value));
    return nextId++;
  }
  return id;
}

bool HashARTDictionary::bulkInsert(size_t size, string* values) {
  for (size_t i = 0; i < size; i++) {
    insert(values[i]);
  }

  return true;
}

bool HashARTDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }

  if (value != it->second) {
    id = insert(value);
  }
  return true;
}

bool HashARTDictionary::lookup(std::string value, uint64_t& id) {
  return reverseIndex.lookup(value, id);
}

bool HashARTDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}
