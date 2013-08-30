#ifndef H_UncompressedDictionary
#define H_UncompressedDictionary

#include <unordered_map>
#include "Dictionary.hpp"

/**
 * Reference dictionary implementation.
 *
 * Represents the (memory-wise) worst case
 * - no compression at all.
 */
class UncompressedDictionary : public Dictionary {
  private:
    typedef std::unordered_map<uint64_t, std::string> IndexType;
    IndexType index;
    typedef std::unordered_map<std::string, uint64_t> ReverseIndexType;
    ReverseIndexType reverseIndex;

  public:
    ~UncompressedDictionary() noexcept { }

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);
    Dictionary::Iterator rangeLookup(std::string prefix);

    std::string name() const {
      return "Hash/Hash (uncompressed)";
    }

    class Iterator : public Dictionary::Iterator {
      private:
        UncompressedDictionary* dict;
        std::string prefix;
        UncompressedDictionary::IndexType::const_iterator iterator;

      public:
        Iterator(UncompressedDictionary* dict, std::string prefix);
        const std::pair<uint64_t, std::string> operator*();
        Dictionary::Iterator& operator++();
        operator bool();
    };

    friend Iterator;
};

#endif
