#include "Hash.hpp"

template<typename TKey>
void Hash<TKey>::insert(TKey key, uint64_t value) {
  index.insert(std::make_pair(key, value));
}

template<typename TKey>
bool Hash<TKey>::lookup(TKey key, uint64_t& value) const {
  auto it = index.find(key);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

template<typename TKey>
std::string Hash<TKey>::description() {
  return "Hash";
}

template class Hash<uint64_t>;
template class Hash<std::string>;
