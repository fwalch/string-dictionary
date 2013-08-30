#ifndef H_BPlusTree
#define H_BPlusTree

#include "stx/btree_map.hpp"
#include <cstdint>
#include <string>

template<typename TKey> class BPlusTree {
  private:
    stx::btree_map<TKey, uint64_t> index;

  public:
    void insert(TKey key, uint64_t value);
    bool lookup(TKey key, uint64_t& value) const;
    static std::string description();
};

#endif
