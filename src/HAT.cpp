#include "Indexes.hpp"

template<typename TKey>
HAT<TKey>::HAT() {
  index = hattrie_create();
}

template<typename TKey>
HAT<TKey>::~HAT() {
  hattrie_free(index);
}

template<typename TKey>
std::string HAT<TKey>::description() {
  return "HAT";
}

template<>
void HAT<std::string>::insert(std::string key, uint64_t value) {
  bool inserted = false;
  uint64_t* valuePtr = hattrie_get(index, key.c_str(), key.size() + 1, &inserted);
  if (inserted) {
    // Key was not in dictionary
    *valuePtr = value;
  }
}

template<>
bool HAT<std::string>::lookup(std::string key, uint64_t& value) const {
  uint64_t* valuePtr = hattrie_tryget(index, key.c_str(), key.size() + 1);
  if (valuePtr == nullptr) {
    return false;
  }
  value = *valuePtr;
  return true;
}

template class HAT<std::string>;
