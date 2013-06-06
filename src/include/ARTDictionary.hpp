#ifndef H_ARTDictionary
#define H_ARTDictionary

#include "IndexART.hpp"
#include "ReverseIndexART.hpp"
#include "Dictionary.hpp"

/**
 * Adaptive Radix Tree dictionary.
 */
class ARTDictionary : public Dictionary {
  private:
    IndexART index;
    IAReverseIndexART reverseIndex;

  public:
    ARTDictionary() : reverseIndex(index) { }
    ~ARTDictionary() noexcept { }

    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "ARTDictionary";
    }
};

#endif
