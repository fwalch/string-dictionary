#ifndef H_HARTDictionary
#define H_HARTDictionary

#include <cassert>
#include "AdaptiveRadixTree.hpp"
#include "hat-trie.h"
#include "Dictionary.hpp"
#include "SingleUncompressedPage.hpp"
#include "MultiUncompressedPage.hpp"
#include "DynamicPage.hpp"

class HARTDictionary;

class ART : public AdaptiveRadixTree {
  private:
    uint8_t lastKey[8];
    HARTDictionary* dict;

  protected:
    uint8_t* loadKey(uintptr_t tid);

  public:
    ART(HARTDictionary* dictionary) : AdaptiveRadixTree(), dict(dictionary) {
    }

    void insert(uint64_t key, uint64_t value) {
      uint8_t swappedKey[8];
      reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

      insertValue(tree, &tree, swappedKey, 0, value);
    }

    bool lookup(uint64_t key, uint64_t& value);
};

/**
 * ART + HAT dictionary.
 */
class HARTDictionary : public Dictionary {
  private:
    ART index; // ID -> string
    hattrie_t* reverseIndex; // string -> ID
    typedef DynamicPage<> PageType;

    uint64_t encodeLeaf(PageType* page, uint16_t deltaNumber);
    PageType::Iterator decodeLeaf(uint64_t leafValue);
    void insertLeaf(PageType* page, uint16_t deltaNumber, page::IdType id, std::string value);

  public:
    HARTDictionary();
    ~HARTDictionary() noexcept;

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "HARTDictionary";
    }

    friend class ART;
};

#endif
