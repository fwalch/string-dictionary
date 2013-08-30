#ifndef H_ART
#define H_ART

#include "ARTBase.hpp"
#include <tuple>

template<class TKey>
class ART : public ARTBase {
  private:
    uint8_t* loadKey(uintptr_t leafValue) const;
    uint8_t* convertKey(TKey key) const;
    unsigned keySize(TKey key) const;

  public:
    ART(LeafStore* leafStore);
    void insert(TKey key, uintptr_t value);
    bool lookup(TKey key, uintptr_t& value) const;
    std::pair<uintptr_t, uintptr_t> rangeLookup(TKey prefix) const;
    static std::string description();
};

#endif
