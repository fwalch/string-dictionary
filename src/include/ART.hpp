#ifndef H_ART
#define H_ART

#include "ARTBase.hpp"
#include <tuple>

template<class TKey>
class ART : public ARTBase {
  protected:
    void loadKey(uintptr_t leafValue, uint8_t* key, unsigned maxKeyLength) const;

  public:
    ART(LeafStore* leafStore);
    virtual ~ART() { }
    virtual void insert(TKey key, uintptr_t value);
    virtual bool lookup(TKey key, uintptr_t& value) const;
    std::pair<uintptr_t, uintptr_t> rangeLookup(TKey prefix) const;
    static std::string description();
};

#endif
