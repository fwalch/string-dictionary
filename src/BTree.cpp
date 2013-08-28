#include "BTree.hpp"

template<typename TKey>
void BTree<TKey>::insert(TKey key, uint64_t value) {
  index[key] = value;
}

template<typename TKey>
bool BTree<TKey>::lookup(TKey key, uint64_t& value) const {
  auto it = index.find(key);

  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

template<typename TKey>
std::string BTree<TKey>::description() {
  return "BTree";
}

template class BTree<uint64_t>;
template class BTree<std::string>;
