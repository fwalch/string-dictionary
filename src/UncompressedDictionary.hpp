#ifndef H_UncompressedDictionary
#define H_UncompressedDictionary

#include <string>
#include <unordered_map>

/**
 * Reference dictionary implementation.
 *
 * Represents the (memory-wise) worst case
 * - no compression at all.
 */
//TODO: extract common base class (with type defintions, nextId field)
class UncompressedDictionary {
  private:
    std::unordered_map<unsigned long, std::string> index;
    std::unordered_map<std::string, unsigned long> reverseIndex;
    unsigned long nextId;

  public:
    void insert(std::string& value);
};

#endif
