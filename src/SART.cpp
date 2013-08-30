#include "SART.hpp"
#ifdef DEBUG
#undef NDEBUG
#include <cassert>
#endif
#include <limits>

template<>
SART<std::string>::SART(LeafStore* leafStore) : ART(leafStore) {
}

template<>
std::string SART<std::string>::description() {
  return "SART";
}

template<>
void SART<std::string>::debug() {
  if (tree != NULL && !isLeaf(tree)) {
    tree->print(0);
  }
}

template<>
bool SART<std::string>::lookup(std::string key, uintptr_t& value) const {
#ifdef DEBUG
  assert(key.size() < std::numeric_limits<unsigned>::max());
#endif
  Node* leaf = sartLookupValue(tree, reinterpret_cast<uint8_t*>(const_cast<char*>(key.c_str())), static_cast<unsigned>(key.size()+1), 0);
  if (leaf == nullNode) {
    return false;
  }
#ifdef DEBUG
  assert(isLeaf(leaf));
#endif
  value = getLeafValue(leaf);
  return true;
}

template class SART<std::string>;
