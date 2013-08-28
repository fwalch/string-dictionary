#include "B+Tree.hpp"

template<typename TKey>
void BPlusTree<TKey>::insert(TKey key, uint64_t value) {
  index.insert(key, value);
}

template<typename TKey>
bool BPlusTree<TKey>::lookup(TKey key, uint64_t& value) const {
  auto it = index.find(key);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

template<typename TKey>
std::string BPlusTree<TKey>::description() {
  return "B+Tree";
}

template class BPlusTree<uint64_t>;
template class BPlusTree<std::string>;
