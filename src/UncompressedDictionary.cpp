#include "UncompressedDictionary.hpp"

using namespace std;

uint64_t UncompressedDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value);

  if (reverseIt == reverseIndex.end()) {
    index[nextId] = value;
    reverseIndex[value] = nextId;
    return nextId++;
  }

  return reverseIt->second;
}

bool UncompressedDictionary::bulkInsert(size_t size, string* values) {
  index.reserve(index.size() + size);
  reverseIndex.reserve(reverseIndex.size() + size);

  for (size_t i = 0; i < size; i++) {
    insert(values[i]);
  }

  return true;
}

bool UncompressedDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  if (it->second != value) {
    id = insert(value);
  }
  return true;
}

bool UncompressedDictionary::lookup(std::string value, uint64_t& id) {
  auto reverseIt = reverseIndex.find(value);

  if (reverseIt == reverseIndex.end()) {
    return false;
  }

  id = reverseIt->second;
  return true;
}

bool UncompressedDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}
