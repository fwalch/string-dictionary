#ifndef H_BPlusTreeDictionary
#define H_BPlusTreeDictionary

#include <string>
#include <unordered_map>
#include "Dictionary.hpp"
#include "stx/btree_map.hpp"

/**
 * Dictionary that uses two B-trees as indexes.
 */
class BPlusTreeDictionary : public Dictionary {
  public:
    struct compare {
      int operator()(const char* a, const char* b) const;
    };

  private:
    stx::btree_map<uint64_t, std::string> index;
    //TODO: when using const char* as key, strange memory usage?
    stx::btree_map<std::string, uint64_t> reverseIndex;

  public:
    ~BPlusTreeDictionary() noexcept { }

    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "B+TreeDictionary";
    }
};

#endif
