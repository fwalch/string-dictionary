#include <cstring>
#include "BPlusTreeDictionary.hpp"

using namespace std;

int BPlusTreeDictionary::compare::operator()(const char* lhs, const char* rhs) const {
  return strcmp(lhs, rhs);
}

uint64_t BPlusTreeDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value.c_str());

  if (reverseIt == reverseIndex.end()) {
    index[nextId] = value;
    reverseIndex[index[nextId].c_str()] = nextId;
    return nextId++;
  }

  return reverseIt->second;
}

bool BPlusTreeDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  if (it->second != value) {
    id = insert(value);
  }
  return true;
}

bool BPlusTreeDictionary::lookup(std::string value, uint64_t& id) {
  auto reverseIt = reverseIndex.find(value.c_str());

  if (reverseIt == reverseIndex.end()) {
    return false;
  }

  id = reverseIt->second;
  return true;
}

bool BPlusTreeDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}
