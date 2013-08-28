#include "RedBlack.hpp"

template<typename TKey>
void RedBlack<TKey>::insert(TKey key, uint64_t value) {
  index.insert(std::make_pair(key, value));
}

template<typename TKey>
bool RedBlack<TKey>::lookup(TKey key, uint64_t& value) const {
  auto it = index.find(key);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

template<typename TKey>
std::string RedBlack<TKey>::description() {
  return "Red-Black-Tree";
}

template class RedBlack<uint64_t>;
template class RedBlack<std::string>;
