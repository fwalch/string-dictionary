#include "boost/algorithm/string.hpp"
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

Dictionary::Iterator HATDictionary::rangeLookup(std::string prefix) {
  return Iterator(*this, hattrie_iter_begin(reverseIndex, true), prefix);
}

inline bool HATDictionary::Iterator::matchPrefix() {
  uint64_t* idPtr = hattrie_iter_val(iterator);
  return boost::starts_with(dict.index[*idPtr], prefix);
}

HATDictionary::Iterator::Iterator(HATDictionary& dictionary, hattrie_iter_t* it, std::string pref) : dict(dictionary), iterator(it), prefix(pref) {
  while (!matchPrefix()) {
    hattrie_iter_next(iterator);
  }
}

HATDictionary::Iterator::~Iterator() {
  hattrie_iter_free(iterator);
}

const std::pair<uint64_t, std::string> HATDictionary::Iterator::operator*() {
  uint64_t* idPtr = hattrie_iter_val(iterator);
  return std::make_pair(
    *idPtr,
    dict.index[*idPtr]
  );
}

HATDictionary::Iterator& HATDictionary::Iterator::operator++() {
  hattrie_iter_next(iterator);
  return *this;
}

HATDictionary::Iterator::operator bool() {
  return !hattrie_iter_finished(iterator) && matchPrefix();
}
