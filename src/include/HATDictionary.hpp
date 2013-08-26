#ifndef H_HATDictionary
#define H_HATDictionary

#include <map>
#include "Dictionary.hpp"
#include "hat-trie.h"

/**
 * HAT trie dictionary.
 */
class HATDictionary : public Dictionary {
  private:
    std::map<uint64_t, std::string> index;
    hattrie_t* reverseIndex;

  public:
    HATDictionary();
    ~HATDictionary() noexcept;

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    std::string name() const {
      return "Hash/HAT";
    }
};

#endif
