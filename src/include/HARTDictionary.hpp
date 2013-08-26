#ifndef H_HARTDictionary
#define H_HARTDictionary

#include <cassert>
#include "AdaptiveRadixTree.hpp"
#include "hat-trie.h"
#include "Dictionary.hpp"
#include "MultiUncompressedPage.hpp"
#include "DynamicPage.hpp"

template<class TPageType>
class HARTDictionary;

template<class TPageType>
class ART : public AdaptiveRadixTree {
  private:
    uint8_t lastKey[8];
    HARTDictionary<TPageType>* dict;

  protected:
    uint8_t* loadKey(uintptr_t leafValue) {
      uint64_t tid = (*dict->decodeLeaf(leafValue)).id;

      reinterpret_cast<uint64_t*>(lastKey)[0] = __builtin_bswap64(tid);
      return lastKey;
    }

  public:
    ART(HARTDictionary<TPageType>* dictionary) : AdaptiveRadixTree(), dict(dictionary) {
    }

    void insert(uint64_t key, uint64_t value) {
      uint8_t swappedKey[8];
      reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

      insertValue(tree, &tree, swappedKey, 0, value);
    }

    bool lookup(uint64_t key, uint64_t& value) {
      uint8_t swappedKey[8];
      reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

      Node* leaf = lookupValue(tree, swappedKey, sizeof(uint64_t), 0);
      if (leaf == nullNode) {
        return false;
      }
      assert(isLeaf(leaf));
      value = getLeafValue(leaf);
      return true;
    }
};

/**
 * ART + HAT dictionary.
 */
template<class TPageType = DynamicPage<32>>
class HARTDictionary : public Dictionary {
  private:
    ART<TPageType> index; // ID -> string
    hattrie_t* reverseIndex; // string -> ID

    uint64_t encodeLeaf(TPageType* page, uint16_t deltaNumber) {
      uint64_t leafValue = reinterpret_cast<uint64_t>(page);
      leafValue = leafValue << 16;
      leafValue |= static_cast<uint64_t>(deltaNumber);

      return leafValue;
    }

    typename TPageType::Iterator decodeLeaf(uint64_t leafValue) {
      TPageType* page = reinterpret_cast<TPageType*>(leafValue >> 16);
      uint16_t deltaNumber = leafValue & 0xFFFF;

      return page->get(deltaNumber);
    }

    void insertLeaf(TPageType* page, uint16_t deltaNumber, page::IdType id, std::string value) {
      uint64_t leafValue = encodeLeaf(page, deltaNumber);

      bool inserted = false;
      uint64_t* valuePtr = hattrie_get(reverseIndex, value.c_str(), value.size()+1, &inserted);
      *valuePtr = leafValue;
      assert(inserted);

      index.insert(id, leafValue);
    }

  public:
    HARTDictionary() : index(this) {
      reverseIndex = hattrie_create();
    }

    ~HARTDictionary() noexcept {
      hattrie_free(reverseIndex);
    }

    bool bulkInsert(size_t size, std::string* values) {
      assert(nextId == 0);

      std::vector<std::pair<uint64_t, std::string>> insertValues;
      insertValues.reserve(size);

      for (size_t i = 0; i < size; i++) {
        insertValues.push_back(make_pair(nextId++, values[i]));
      }

      typename TPageType::Loader loader;
      typename TPageType::Loader::CallbackType callback;
      callback = std::bind(&HARTDictionary::insertLeaf, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
      loader.load(insertValues, callback);

      return true;
    }

    uint64_t insert(std::string value) {
      bool inserted = false;
      uint64_t* idPtr = hattrie_get(reverseIndex, value.c_str(), value.size() + 1, &inserted);
      if (inserted) {
        // Create leaf

        //TODO
        /**idPtr = addressOfOurLeaf;
          index.insert(nextId, addressOfOurLeaf);*/
        return nextId++;
      }

      return *idPtr;
    }

    bool update(uint64_t& id, std::string value) {
      uint64_t leafPtr;
      if (!index.lookup(id, leafPtr)) {
        return false;
      }

      TPageType* page = *reinterpret_cast<TPageType**>(leafPtr);
      auto iterator = page->getId(id);
      assert(iterator);

      if (value != (*iterator).value) {
        id = insert(value);
      }
      return true;
    }

    bool lookup(std::string value, uint64_t& id) {
      uint64_t* leafPtr = hattrie_tryget(reverseIndex, value.c_str(), value.size() + 1);
      if (leafPtr == NULL) {
        return false;
      }

      auto iterator = decodeLeaf(*leafPtr);
      assert(iterator);

      id = (*iterator).id;
      return true;
    }

    bool lookup(uint64_t id, std::string& value) {
      uint64_t leafValue;
      if (index.lookup(id, leafValue)) {
        auto iterator = decodeLeaf(leafValue);
        assert(iterator);

        value = (*iterator).value;
        return true;
      }
      return false;
    }

    std::string name() const {
      return "ART/HAT with " + TPageType::name();
    }

    friend class ART<TPageType>;
};

#endif
