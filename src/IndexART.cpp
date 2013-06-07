#include "IndexART.hpp"
#include <cassert>

using namespace std;

IndexART::IndexART() : AdaptiveRadixTree() {
}

void IndexART::loadKey(uintptr_t tid, uint8_t key[]) {
   // Store the key of the tuple into the key vector
   // Implementation is database specific
   reinterpret_cast<uint64_t*>(key)[0] = __builtin_bswap64(tid);
}

void IndexART::insert(uint64_t key, string value) {
  uint8_t keyArray[8];
  loadKey(key, keyArray);
  insertValue(tree, &tree, keyArray, 0, key, 8);
  values.push_back(value);
}

bool IndexART::lookup(uint64_t key, string& value) {
  uint8_t keyArray[8];
  loadKey(key, keyArray);
  Node* leaf = lookupValue(tree, keyArray, 8, 0, 8);
  if (leaf == nullNode) {
    return false;
  }
  assert(isLeaf(leaf));
  value = values.at(static_cast<unsigned long>(getLeafValue(leaf)));
  return true;
}

