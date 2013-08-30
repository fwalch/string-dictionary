#ifndef H_SART
#define H_SART

#include "ART.hpp"
#include <tuple>

template<class TKey>
class SART : public ART<TKey> {
  protected:
    ARTBase::Node* lookupPrefix(ARTBase::Node* node, uint8_t prefix[], unsigned prefixLength, unsigned depth) const {
      return ARTBase::sartLookupPrefix(node, prefix, prefixLength, depth);
    }
  public:
    SART(LeafStore* leafStore);
    bool lookup(TKey key, uintptr_t& value) const;
    static std::string description();
    void debug();
};

#endif
