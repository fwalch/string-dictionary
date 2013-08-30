#include "Indexes.hpp"
#include "boost/algorithm/string.hpp"
#include <iostream>

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

template<>
std::pair<uint64_t, uint64_t> HAT<std::string>::rangeLookup(std::string prefix) const {
  uint64_t start = 0;
  uint64_t end = 0;

  //TODO
  hattrie_iter_t* it = hattrie_iter_begin(index, true);
  while (!hattrie_iter_finished(it)) {
    uint64_t length;
    const char* value = hattrie_iter_key(it, &length);
    if (length >= prefix.size()) {
      bool match = boost::starts_with(value, prefix);
      if (start == 0 && match) {
        start = *hattrie_iter_val(it);
      }

      if (start != 0) {
        if (match) {
          end = *hattrie_iter_val(it);
        }
        else {
          break;
        }
      }
    }
    hattrie_iter_next(it);
  }

  hattrie_iter_free(it);

  return std::make_pair(start, end);
}

template class HAT<std::string>;
