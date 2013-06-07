#ifndef H_IndexART
#define H_IndexART

#include <string>
#include <vector>
#include "AdaptiveRadixTree.hpp"

template<typename TLookup> class ReverseIndexART;

class IndexART : public AdaptiveRadixTree {
  private:
    std::vector<std::string> values;
    uint8_t lastKey[8];

  protected:
    uint8_t* loadKey(uintptr_t tid);

  public:
    IndexART();
    void insert(uint64_t key, std::string value);
    bool lookup(uint64_t key, std::string& value);

  friend class ReverseIndexART<IndexART>;
};

#endif
