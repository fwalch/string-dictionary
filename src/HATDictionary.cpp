#include <cassert>
#include "HATDictionary.hpp"

using namespace std;

HATDictionary::HATDictionary() {
  reverseIndex = hattrie_create();
}

HATDictionary::~HATDictionary() noexcept {
  hattrie_free(reverseIndex);
}

uint64_t HATDictionary::insert(string value) {
  bool inserted = false;
  uint64_t* idPtr = hattrie_get(reverseIndex, value.c_str(), value.size() + 1, &inserted);
  if (inserted) {
    // String not in dictionary
    *idPtr = nextId;
    index.insert(make_pair(nextId, value));
    return nextId++;
  }

  return *idPtr;
}

bool HATDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  for (; nextId < size; nextId++) {
    hattrie_get(reverseIndex, values[nextId].c_str(), values[nextId].size() + 1, NULL);
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
  uint64_t* idPtr = hattrie_tryget(reverseIndex, value.c_str(), value.size() + 1);
  if (idPtr == NULL) {
    return false;
  }
  id = *idPtr;
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
