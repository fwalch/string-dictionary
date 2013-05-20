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
    typedef std::unordered_map<IdType, std::string> IndexType;
    IndexType index;
    UMReverseIndexART reverseIndex;

  public:
    HashARTDictionary() : reverseIndex(index) { }
    ~HashARTDictionary() noexcept { }
    void insert(std::string value);
};

#endif
