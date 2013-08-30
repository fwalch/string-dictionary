#include "boost/algorithm/string.hpp"
#ifdef DEBUG
#include <cassert>
#endif
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

void SimpleDictionary::bulkInsert(size_t size, string* values) {
#ifdef DEBUG
  assert(nextId == 1);
#endif

  index.reserve(size);
  reverseIndex.reserve(size);

  for (; nextId < size; nextId++) {
    size_t len = values[nextId].length() + 1;
    char* insertValue = new char[len];
    memcpy(insertValue, values[nextId].c_str(), len);
    index[nextId] = insertValue;
    reverseIndex[insertValue] = nextId;
  }
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

bool SimpleDictionary::lookup(std::string value, uint64_t& id) const {
  auto reverseIt = reverseIndex.find(value.c_str());

  if (reverseIt == reverseIndex.end()) {
    return false;
  }

  id = reverseIt->second;
  return true;
}

bool SimpleDictionary::lookup(uint64_t id, std::string& value) const {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void SimpleDictionary::rangeLookup(std::string prefix, RangeLookupCallbackType callback) const {
  throw;
}
#pragma GCC diagnostic pop

size_t SimpleDictionary::hash::operator()(const char* value) const {
  return std::hash<string>()(value);
}

bool SimpleDictionary::equal_to::operator()(const char* lhs, const char* rhs) const {
  return strcmp(lhs, rhs) == 0;
}
