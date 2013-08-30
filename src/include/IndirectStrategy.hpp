#ifndef H_IndirectStrategy
#define H_IndirectStrategy

#include "StrategyBase.hpp"

template<class TIdIndex, class TStringIndex, class TLeaf>
class IndirectStrategy : public StrategyBase<TIdIndex, TStringIndex, TLeaf> {
  public:
    IndirectStrategy(TIdIndex& idIndex, TStringIndex& strIndex) : StrategyBase<TIdIndex, TStringIndex, TLeaf>(idIndex, strIndex) {
    }

    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue) const;
    void leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint16_t offset, uint64_t id, std::string value);

    //TODO: why!??
    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, uint64_t lookupId) const {
      return StrategyBase<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(leafValue, lookupId);
    }

    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, std::string lookupValue) const {
      return StrategyBase<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(leafValue, lookupValue);
    }
};

#include "../IndirectStrategy.cpp"

#endif
