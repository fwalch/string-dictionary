#include "Indexes.hpp"

template<typename TKey>
ART<TKey>::ART(LeafStore* leafStore) : index(leafStore) {
}

template<typename TKey>
void ART<TKey>::insert(TKey key, uint64_t value) {
  index.insert(key, value);
}

template<typename TKey>
bool ART<TKey>::lookup(TKey key, uint64_t& value) const {
  return index.lookup(key, value);
}

template<typename TKey>
std::string ART<TKey>::description() {
  return "ART";
}

template class ART<std::string>;
template class ART<uint64_t>;
