#include "ReverseIndexART.hpp"
#include <cassert>
#include <cstring>

using namespace std;
void IAReverseIndexART::loadKey(uintptr_t tid, uint8_t keyArray[]) {
  std::string key = index.values.at(tid);
  memcpy(keyArray, key.c_str(), key.size());
}

void UMReverseIndexART::loadKey(uintptr_t tid, uint8_t keyArray[]) {
  std::string key = index.at(tid);
  memcpy(keyArray, key.c_str(), key.size());
}

void IAReverseIndexART::insert(std::string key, uint64_t value) {
  unsigned strlen = static_cast<unsigned>(key.size());
  if (strlen > maxKeyLength) {
    maxKeyLength = strlen;
  }
  uint8_t* keyArray = new uint8_t[strlen];
  memcpy(keyArray, key.c_str(), strlen);
  insertValue(tree, &tree, keyArray, 0, value, maxKeyLength);
  delete[] keyArray;
}
bool IAReverseIndexART::lookup(std::string key, uint64_t& value) {
  unsigned strlen = static_cast<unsigned>(key.size());
  uint8_t* keyArray = new uint8_t[strlen];
  memcpy(keyArray, key.c_str(), strlen);
  Node* leaf = lookupValue(tree, keyArray, strlen, 0, maxKeyLength);
  if (leaf == nullNode) {
    delete[] keyArray;
    return false;
  }
  assert(isLeaf(leaf));
  value = static_cast<unsigned long>(getLeafValue(leaf));
  delete[] keyArray;
  return true;
}

void UMReverseIndexART::insert(std::string key, uint64_t value) {
  unsigned strlen = static_cast<unsigned>(key.size());
  if (strlen > maxKeyLength) {
    maxKeyLength = strlen;
  }
  uint8_t* keyArray = new uint8_t[strlen];
  memcpy(keyArray, key.c_str(), strlen);
  insertValue(tree, &tree, keyArray, 0, value, maxKeyLength);
  delete[] keyArray;
}

bool UMReverseIndexART::lookup(std::string key, uint64_t& value) {
  unsigned strlen = static_cast<unsigned>(key.size());
  uint8_t* keyArray = new uint8_t[strlen];
  memcpy(keyArray, key.c_str(), strlen);
  Node* leaf = lookupValue(tree, keyArray, strlen, 0, maxKeyLength);
  if (leaf == nullNode) {
    delete[] keyArray;
    return false;
  }
  assert(isLeaf(leaf));
  value = static_cast<unsigned long>(getLeafValue(leaf));
  delete[] keyArray;
  return true;
}
