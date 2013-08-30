#include "boost/algorithm/string.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include "SimpleDictionary.hpp"

using namespace std;

SimpleDictionary::~SimpleDictionary() noexcept {
  for (auto it : reverseIndex) {
    delete[] it.first;
  }
}

uint64_t SimpleDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value.c_str());

  if (reverseIt == reverseIndex.end()) {
    size_t len = value.length() + 1;
    char* insertValue = new char[len];
    memcpy(insertValue, value.c_str(), len);
    index[nextId] = insertValue;
    reverseIndex[insertValue] = nextId;
    return nextId++;
  }

  return reverseIt->second;
}

bool SimpleDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  index.reserve(size);
  reverseIndex.reserve(size);

  for (; nextId < size; nextId++) {
    size_t len = values[nextId].length() + 1;
    char* insertValue = new char[len];
    memcpy(insertValue, values[nextId].c_str(), len);
    index[nextId] = insertValue;
    reverseIndex[insertValue] = nextId;
  }

  return true;
}

bool SimpleDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  if (it->second != value) {
    id = insert(value);
  }
  return true;
}

bool SimpleDictionary::lookup(std::string value, uint64_t& id) {
  auto reverseIt = reverseIndex.find(value.c_str());

  if (reverseIt == reverseIndex.end()) {
    return false;
  }

  id = reverseIt->second;
  return true;
}

bool SimpleDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

Dictionary::Iterator SimpleDictionary::rangeLookup(std::string prefix) {
  return Iterator(this, prefix);
}

SimpleDictionary::Iterator::Iterator(SimpleDictionary* dictionary, std::string pref) : dict(dictionary), prefix(pref), iterator(dictionary->index.cbegin()) {
}

SimpleDictionary::Iterator& SimpleDictionary::Iterator::operator++() {
  ++iterator;
  return *this;
}

SimpleDictionary::Iterator::operator bool() {
  return iterator != dict->index.cend()
    && boost::starts_with(iterator->second, prefix);
}

const std::pair<uint64_t, std::string> SimpleDictionary::Iterator::operator*() {
  return std::make_pair(iterator->first, iterator->second);
}

size_t SimpleDictionary::hash::operator()(const char* value) const {
  return std::hash<string>()(value);
}

bool SimpleDictionary::equal_to::operator()(const char* lhs, const char* rhs) const {
  return strcmp(lhs, rhs) == 0;
}
