#include <functional>
#include "SimpleDictionary.hpp"
#include <cstring>

using namespace std;

SimpleDictionary::~SimpleDictionary() noexcept {
  for (auto it : reverseIndex) {
    delete[] it.first;
  }
}

void SimpleDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value.c_str());
  if (reverseIt == reverseIndex.end()) {
    // String not in dictionary
    char* insertValue = new char[value.length() + 1];
    memcpy(insertValue, value.c_str(), value.length() + 1);
    index[nextId] = insertValue;
    reverseIndex[insertValue] = nextId;
    nextId++;
  }
}

size_t SimpleDictionary::hash::operator()(const char* value) const {
  return std::hash<string>()(value);
}

bool SimpleDictionary::equal_to::operator()(const char* lhs, const char* rhs) const {
  return strcmp(lhs, rhs) == 0;
}
