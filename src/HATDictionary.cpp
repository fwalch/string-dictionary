#include <cassert>
#include "HATDictionary.hpp"

using namespace std;

uint64_t HATDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value);
  if (reverseIt == reverseIndex.end()) {
    // String not in dictionary
    reverseIndex.insert(value);
    index.insert(make_pair(nextId, value));
    return nextId++;
  }

  // TODO: we cannot determine the ID because we only have a set
  return 0;
}

bool HATDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  reverseIndex.insert(values, values + size);

  for (; nextId < size; nextId++) {
    index.insert(make_pair(nextId, values[nextId]));
  }

  return true;
}

bool HATDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }

  if (it->second != value) {
    id = insert(value);
  }

  return true;
}

bool HATDictionary::lookup(std::string value, uint64_t& id) {
  auto reverseIt = reverseIndex.find(value);
  if (reverseIt == reverseIndex.end()) {
    return false;
  }

  // TODO: we cannot determine the ID because we only have a set
  id = 0;
  return true;
}

bool HATDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }
  value = it->second;
  return true;
}
