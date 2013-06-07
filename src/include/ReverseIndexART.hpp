#ifndef H_ReverseIndexART
#define H_ReverseIndexART

#include <string>
#include <cstring>
#include <cassert>
#include "AdaptiveRadixTree.hpp"

template<typename TLookup>
class ReverseIndexART : public AdaptiveRadixTree {
  private:
    uint32_t maxKeyLength;
    TLookup& index;
    uint8_t* loadKey(uintptr_t tid);
    uint8_t* cast(std::string str) {
      return reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str()));
    }

  public:
    ReverseIndexART(TLookup& lookup) : AdaptiveRadixTree(), maxKeyLength(0), index(lookup) { }

    void insert(std::string key, uint64_t value) {
      unsigned strlen = static_cast<unsigned>(key.size() + 1);
      if (strlen > maxKeyLength) {
        maxKeyLength = strlen;
      }

      insertValue(tree, &tree, cast(key), 0, value);
    }

    bool lookup(std::string key, uint64_t& value) {
      unsigned strlen = static_cast<unsigned>(key.size() + 1);
      Node* leaf = lookupValue(tree, cast(key), strlen, 0);

      if (leaf == NULL) {
        return false;
      }

      assert(isLeaf(leaf));
      value = static_cast<uint64_t>(getLeafValue(leaf));

      return true;
    }
};

#endif
