#ifndef H_ART
#define H_ART

#include "AdaptiveRadixTree.hpp"

template<typename TKey> class ART {
  private:
    AdaptiveRadixTree<TKey> index;

  public:
    ART(LeafStore* leafStore);
    void insert(TKey key, uint64_t value);
    bool lookup(TKey key, uint64_t& value) const;
    static std::string description();
};

#endif
