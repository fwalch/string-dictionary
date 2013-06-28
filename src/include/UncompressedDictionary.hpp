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

    const char* name() const {
      return "UncompressedDictionary";
    }
};

#endif
