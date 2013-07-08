#include <cassert>
#include <cstring>
#include <emmintrin.h>
#include "ARTcDictionary.hpp"

using namespace std;

int delete_leaf(void* data, const char* key, uint32_t key_len, void* val);

ARTcDictionary::ARTcDictionary() {
  assert(init_art_tree(&index) == 0);
  assert(init_art_tree(&reverseIndex) == 0);
}

ARTcDictionary::~ARTcDictionary() noexcept {
  // Delete leaves
  art_iter(&reverseIndex, delete_leaf, NULL);

  assert(destroy_art_tree(&index) == 0);
  assert(destroy_art_tree(&reverseIndex) == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int delete_leaf(void* data, const char* key, uint32_t key_len, void* val) {
  delete[] reinterpret_cast<char*>(val);
  return 0;
}
#pragma GCC diagnostic pop

inline uint64_t unpack_id(char* leaf) {
  return reinterpret_cast<uint64_t*>(leaf)[0];
}

inline const char* unpack_value(char* leaf) {
  return const_cast<const char*>(leaf + 8);
}

inline char* make_leaf(uint64_t id, const char* value, size_t len) {
  char* leaf = new char[sizeof(uint64_t) + len];
  reinterpret_cast<uint64_t*>(leaf)[0] = id;
  memcpy(leaf + sizeof(uint64_t), value, len);
  return leaf;
}

uint64_t ARTcDictionary::insert(string value) {
  int len = static_cast<int>(value.size()) + 1;

  char* foundLeaf;
  if ((foundLeaf = reinterpret_cast<char*>(art_search(&reverseIndex, const_cast<char*>(value.c_str()), len))) == NULL) {
    // String not in dictionary
    char* leaf = make_leaf(nextId, value.c_str(), static_cast<size_t>(len));
    assert(art_insert(&reverseIndex, const_cast<char*>(value.c_str()), len, leaf) == NULL);

    char idKey[sizeof(uint64_t)];
    reinterpret_cast<uint64_t*>(idKey)[0] = __builtin_bswap64(nextId);
    assert(art_insert(&index, idKey, sizeof(uint64_t), leaf) == NULL);
    return nextId++;
  }
  return unpack_id(foundLeaf);
}

bool ARTcDictionary::bulkInsert(size_t size, string* values) {
  for (size_t i = 0; i < size; i++) {
    insert(values[i]);
  }

  return true;
}

bool ARTcDictionary::update(uint64_t& id, string value) {
  char* leaf;

  char idKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(idKey)[0] = __builtin_bswap64(id);
  if ((leaf = reinterpret_cast<char*>(art_search(&index, idKey, sizeof(uint64_t)))) == NULL) {
    return false;
  }

  if (strncmp(unpack_value(leaf), value.c_str(), value.length() + 1) != 0) {
    id = insert(value);
  }
  return true;
}

bool ARTcDictionary::lookup(string value, uint64_t& id) {
  char* leaf = reinterpret_cast<char*>(art_search(&reverseIndex, const_cast<char*>(value.c_str()), static_cast<int>(value.length()) + 1));
  if (leaf == NULL) {
    return false;
  }
  id = unpack_id(leaf);
  return true;
}

bool ARTcDictionary::lookup(uint64_t id, string& value) {
  char idKey[sizeof(uint64_t)];
  reinterpret_cast<uint64_t*>(idKey)[0] = __builtin_bswap64(id);
  char* leaf = reinterpret_cast<char*>(art_search(&index, idKey, sizeof(uint64_t)));
  if (leaf == NULL) {
    return false;
  }
  value = string(unpack_value(leaf));
  return true;
}
