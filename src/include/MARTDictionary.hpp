#ifndef H_MARTDictionary
#define H_MARTDictionary

#include "IndexART.hpp"
#include "ReverseIndexMART.hpp"
#include "Dictionary.hpp"

/**
 * Adaptive Radix Tree dictionary.
 */
class MARTDictionary : public Dictionary {
  private:
    IndexART index;
    ReverseIndexMART reverseIndex;

  public:
    MARTDictionary() : reverseIndex(index) { }
    ~MARTDictionary() noexcept { }

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "MARTDictionary";
    }
};

#endif
