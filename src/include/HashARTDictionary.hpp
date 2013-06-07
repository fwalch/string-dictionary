#ifndef H_HashARTDictionary
#define H_HashARTDictionary

#include <string>
#include <unordered_map>
#include "ReverseIndexART.hpp"
#include "Dictionary.hpp"

/**
 * Adaptive Radix Tree dictionary with a hashmap.
 *
 * The hashmap is used for the ID -> string lookup,
 * the ART for string -> ID.
 */
class HashARTDictionary : public Dictionary {
  private:
    typedef std::unordered_map<uint64_t, std::string> IndexType;
    IndexType index;
    ReverseIndexART<IndexType> reverseIndex;

  public:
    HashARTDictionary() : reverseIndex(index) { }
    ~HashARTDictionary() noexcept { }

    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "HashARTDictionary";
    }
};

#endif
