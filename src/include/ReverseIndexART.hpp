#ifndef H_ReverseIndexART
#define H_ReverseIndexART

#include <string>
#include <unordered_map>
#include <cstring>
#include <cassert>
#include "AdaptiveRadixTree.hpp"
#include "IndexART.hpp"

/*template<typename TLookup>
class ReverseIndexART : public AdaptiveRadixTree {
  private:
    uint32_t maxKeyLength;
    TLookup& index;

  protected:
    void loadKey(uintptr_t tid, uint8_t keyArray[]);

  public:
    ReverseIndexART(TLookup& lookup) : AdaptiveRadixTree(), maxKeyLength(0), index(lookup) { }
    void insert(std::string key, uint64_t value);
    bool lookup(std::string key, uint64_t& value);
};*/

//TODO
class IAReverseIndexART : public AdaptiveRadixTree {
  private:
    uint32_t maxKeyLength;
    IndexART& index;

  protected:
    void loadKey(uintptr_t tid, uint8_t keyArray[]);

  public:
    IAReverseIndexART(IndexART& lookup) : AdaptiveRadixTree(), maxKeyLength(0), index(lookup) { }
    void insert(std::string key, uint64_t value);
    bool lookup(std::string key, uint64_t& value);
};

class UMReverseIndexART : public AdaptiveRadixTree {
  private:
    uint32_t maxKeyLength;
    std::unordered_map<uintptr_t, std::string>& index;

  protected:
    void loadKey(uintptr_t tid, uint8_t keyArray[]);

  public:
    UMReverseIndexART(std::unordered_map<uintptr_t, std::string>& lookup) : AdaptiveRadixTree(), maxKeyLength(0), index(lookup) { }
    void insert(std::string key, uint64_t value);
    bool lookup(std::string key, uint64_t& value);
};

#endif
