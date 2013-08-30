#include "ART.hpp"
#ifdef DEBUG
#undef NDEBUG
#include <cassert>
#endif
#include <limits>
#include <cstring>
#include <memory>

// Generic implementations

template<typename TKey>
ART<TKey>::ART(LeafStore* leafStore) : ARTBase(leafStore) {
}

template<typename TKey>
std::string ART<TKey>::description() {
  return "ART";
}


// uint64_t implementations

template<>
bool ART<uint64_t>::lookup(uint64_t key, uintptr_t& value) const {
  uint8_t swappedKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  Node* leaf = lookupValue(tree, swappedKey, sizeof(uint64_t), 0);
  if (leaf == nullNode) {
    return false;
  }
#ifdef DEBUG
  assert(isLeaf(leaf));
#endif
  value = getLeafValue(leaf);
  return true;
}

template<>
void ART<uint64_t>::insert(uint64_t key, uint64_t value) {
  uint8_t swappedKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  insertValue(tree, &tree, swappedKey, 0, value, sizeof(uint64_t));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<>
inline void ART<uint64_t>::loadKey(uintptr_t leafValue, uint8_t* key, unsigned maxKeyLength) const {
#ifdef DEBUG
  assert(maxKeyLength == sizeof(uint64_t));
#endif
  uint64_t id = leafStore->getId(leafValue);
  reinterpret_cast<uint64_t*>(key)[0]=__builtin_bswap64(id);
}
#pragma GCC diagnostic pop

// std::string implementations

template<>
bool ART<std::string>::lookup(std::string key, uintptr_t& value) const {
#ifdef DEBUG
  assert(key.size() < std::numeric_limits<unsigned>::max());
#endif
  Node* leaf = lookupValue(tree, reinterpret_cast<uint8_t*>(const_cast<char*>(key.c_str())), static_cast<unsigned>(key.size()+1), 0);
  if (leaf == nullNode) {
    return false;
  }
#ifdef DEBUG
  assert(isLeaf(leaf));
#endif
  value = getLeafValue(leaf);
  return true;
}

template<>
void ART<std::string>::insert(std::string key, uint64_t value) {
#ifdef DEBUG
  assert(key.size() < std::numeric_limits<unsigned>::max());
#endif

  insertValue(tree, &tree, reinterpret_cast<uint8_t*>(const_cast<char*>(key.c_str())), 0, value, static_cast<unsigned>(key.size()+1));
}

template<>
inline void ART<std::string>::loadKey(uintptr_t leafValue, uint8_t* key, unsigned maxKeyLength) const {
  std::string value = leafStore->getValue(leafValue);
  strncpy(reinterpret_cast<char*>(key), value.c_str(), maxKeyLength);
}

template<>
std::pair<uintptr_t, uintptr_t> ART<std::string>::rangeLookup(std::string prefix) const {
#ifdef DEBUG
  assert(prefix.size() <= std::numeric_limits<unsigned>::max());
#endif

  // Use only prefix.size() to exclude NULL terminator!
  Node* prefixNode = lookupPrefix(tree, reinterpret_cast<uint8_t*>(const_cast<char*>((prefix.c_str()))), static_cast<unsigned>(prefix.size()), 0);

  return std::make_pair(
      getLeafValue(minimum(prefixNode)),
      getLeafValue(maximum(prefixNode)));
}

template class ART<uint64_t>;
template class ART<std::string>;
