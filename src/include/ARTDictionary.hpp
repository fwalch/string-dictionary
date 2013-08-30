#ifndef H_ARTDictionary
#define H_ARTDictionary

#include "IndexART.hpp"
#include "ReverseIndexART.hpp"
#include "Dictionary.hpp"
#include <stack>

/**
 * Adaptive Radix Tree dictionary.
 */
class ARTDictionary : public Dictionary {
  private:
    IndexART index;
    ReverseIndexART<IndexART> reverseIndex;

  public:
    ARTDictionary() : reverseIndex(index) { }
    ~ARTDictionary() noexcept { }

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);
    Dictionary::Iterator rangeLookup(std::string prefix);

    std::string name() const {
      return "ART/ART";
    }

    class Iterator : public Dictionary::Iterator {
      private:
        std::stack<AdaptiveRadixTree::Node*> stack;
        ARTDictionary* dict;
        std::string prefix;

      public:
        Iterator(ARTDictionary* dict, std::string prefix);
        const std::pair<uint64_t, std::string> operator*();
        Dictionary::Iterator& operator++();
        operator bool();
    };

    friend Iterator;
};

#endif
