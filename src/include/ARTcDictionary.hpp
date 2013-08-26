#ifndef H_ARTcDictionary
#define H_ARTcDictionary

#include "Dictionary.hpp"
#include "art.h"
#include <vector>

/**
 * Adaptive Radix Tree dictionary.
 */
class ARTcDictionary : public Dictionary {
  private:
    art_tree index;
    art_tree reverseIndex;

  public:
    ARTcDictionary();
    ~ARTcDictionary() noexcept;

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    std::string name() const {
      return "ART/ART (libart)";
    }
};

#endif
