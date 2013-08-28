#ifndef H_HAT
#define H_HAT

#include "hat-trie.h"

template<typename TKey> class HAT {
  private:
    hattrie_t* index;

  public:
    HAT();
    ~HAT();
    void insert(TKey key, uint64_t value);
    bool lookup(TKey key, uint64_t& value) const;
    static std::string description();
};

#endif
