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

  protected:
    void loadKey(uintptr_t tid, uint8_t keyArray[]);

  public:
    ReverseIndexART(TLookup& lookup) : AdaptiveRadixTree(), maxKeyLength(0), index(lookup) { }

    void insert(std::string key, uint64_t value) {
      unsigned strlen = static_cast<unsigned>(key.size() + 1);
      if (strlen > maxKeyLength) {
        maxKeyLength = strlen;
      }
      uint8_t* keyArray = new uint8_t[strlen];
      memcpy(keyArray, key.c_str(), strlen);
      insertValue(tree, &tree, keyArray, 0, value, maxKeyLength);
      delete[] keyArray;
    }

    bool lookup(std::string key, uint64_t& value) {
      unsigned strlen = static_cast<unsigned>(key.size() + 1);
      uint8_t* keyArray = new uint8_t[strlen];
      memcpy(keyArray, key.c_str(), strlen);
      Node* leaf = lookupValue(tree, keyArray, strlen, 0, maxKeyLength);
      if (leaf == nullNode) {
        delete[] keyArray;
        return false;
      }
      assert(isLeaf(leaf));
      value = static_cast<unsigned long>(getLeafValue(leaf));
      delete[] keyArray;
      return true;
    }
};

#endif
