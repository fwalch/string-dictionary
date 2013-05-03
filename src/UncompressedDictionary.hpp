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
    std::unordered_map<IdType, std::string> index;
    std::unordered_map<std::string, IdType> reverseIndex;

  public:
    ~UncompressedDictionary() noexcept { }
    void insert(std::string value);
};

#endif
