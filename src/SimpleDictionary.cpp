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
    char* insertValue = new char[value.length() + 1];
    memcpy(insertValue, value.c_str(), value.length() + 1);
    index[nextId] = insertValue;
    reverseIndex[insertValue] = nextId;
    return nextId++;
  }

  return reverseIt->second;
}

bool SimpleDictionary::bulkInsert(size_t size, string* values) {
  index.reserve(index.size() + size);
  reverseIndex.reserve(reverseIndex.size() + size);

  for (size_t i = 0; i < size; i++) {
    insert(values[i]);
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

size_t SimpleDictionary::hash::operator()(const char* value) const {
  return std::hash<string>()(value);
}

bool SimpleDictionary::equal_to::operator()(const char* lhs, const char* rhs) const {
  return strcmp(lhs, rhs) == 0;
}
