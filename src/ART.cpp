#include "ART.hpp"
#include <cassert>
#include <limits>

// Generic implementations

template<typename TKey>
ART<TKey>::ART(LeafStore* leafStore) : ARTBase(leafStore) {
}

template<typename TKey>
void ART<TKey>::insert(TKey key, uintptr_t value) {
  insertValue(tree, &tree, convertKey(key), 0, value);
}

template<typename TKey>
bool ART<TKey>::lookup(TKey key, uintptr_t& value) const {
  Node* leaf = lookupValue(tree, convertKey(key), keySize(key), 0);
  if (leaf == nullNode) {
    return false;
  }
  assert(isLeaf(leaf));
  value = getLeafValue(leaf);
  return true;
}

template<typename TKey>
std::string ART<TKey>::description() {
  return "ART";
}


// uint64_t implementations

template<>
inline uint8_t* ART<uint64_t>::convertKey(uint64_t key) const {
  static uint8_t swappedKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(const_cast<uint8_t*>(swappedKey))[0] = __builtin_bswap64(key);
  return swappedKey;
}

template<>
bool ART<uint64_t>::lookup(uint64_t key, uintptr_t& value) const {
  uint8_t swappedKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  Node* leaf = lookupValue(tree, swappedKey, sizeof(uint64_t), 0);
  if (leaf == nullNode) {
    return false;
  }
  assert(isLeaf(leaf));
  value = getLeafValue(leaf);
  return true;
}

template<>
void ART<uint64_t>::insert(uint64_t key, uint64_t value) {
  uint8_t swappedKey[8];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  insertValue(tree, &tree, swappedKey, 0, value);
}

template<>
inline uint8_t* ART<uint64_t>::loadKey(uintptr_t leafValue) const {
  return convertKey(leafStore->getLeaf(leafValue).first);
}

// std::string implementations

template<>
inline uint8_t* ART<std::string>::convertKey(std::string value) const {
  return reinterpret_cast<uint8_t*>(const_cast<char*>(value.c_str()));
}

template<>
inline unsigned ART<std::string>::keySize(std::string value)  const {
  assert(value.size() < std::numeric_limits<unsigned>::max());
  return static_cast<unsigned>(value.size())+1;
}

template<>
inline uint8_t* ART<std::string>::loadKey(uintptr_t leafValue) const {
  return convertKey(leafStore->getLeaf(leafValue).second);
}

#include <iostream>
template<>
std::pair<uintptr_t, uintptr_t> ART<std::string>::rangeLookup(std::string prefix) const {
  Node* prefixNode = lookupPrefix(tree, convertKey(prefix), keySize(prefix)-1, 0);

  return std::make_pair(
    getLeafValue(minimum(prefixNode)),
    getLeafValue(maximum(prefixNode)));
}

template class ART<uint64_t>;
template class ART<std::string>;