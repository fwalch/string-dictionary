#ifndef H_BTreeDictionary
#define H_BTreeDictionary

#include <string>
#include <unordered_map>
#include "Dictionary.hpp"
#include "btree/btree_map.hpp"

/**
 * Dictionary that uses two B-trees as indexes.
 */
class BTreeDictionary : public Dictionary {
  public:
    struct compare : public btree::btree_key_compare_to_tag {
      int operator()(const char* a, const char* b) const;
    };

  private:
    btree::btree_map<uint64_t, std::string> index;
    btree::btree_map<const char*, uint64_t, compare> reverseIndex;

  public:
    ~BTreeDictionary() noexcept { }

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    std::string name() const {
      return "B-tree/B-tree";
    }
};

#endif
