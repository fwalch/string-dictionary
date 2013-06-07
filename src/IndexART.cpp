#include <cassert>
#include <cstring>
#include "IndexART.hpp"

using namespace std;

IndexART::IndexART() : AdaptiveRadixTree() {
}

uint8_t* IndexART::loadKey(uintptr_t tid) {
  reinterpret_cast<uint64_t*>(lastKey)[0] = __builtin_bswap64(tid);
  return lastKey;
}

void IndexART::insert(uint64_t key, string value) {
  uint8_t swappedKey[8];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  //TODO: key must be reconstructible from value
  insertValue(tree, &tree, swappedKey, 0, key);
  values.push_back(value);
}

bool IndexART::lookup(uint64_t key, string& value) {
  Node* leaf = lookupValue(tree, loadKey(key), 8, 0);
  if (leaf == nullNode) {
    return false;
  }
  assert(isLeaf(leaf));
  value = values.at(static_cast<unsigned long>(getLeafValue(leaf)));
  return true;
}

