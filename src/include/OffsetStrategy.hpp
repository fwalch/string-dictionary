#ifndef H_OffsetStrategy
#define H_OffsetStrategy

#include "StrategyBase.hpp"

template<class TIdIndex, class TStringIndex, class TLeaf>
class OffsetStrategy : public StrategyBase<TIdIndex, TStringIndex, TLeaf> {
  public:
    OffsetStrategy(TIdIndex& idIndex, TStringIndex& strIndex) : StrategyBase<TIdIndex, TStringIndex, TLeaf>(idIndex, strIndex) {
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

#include "../OffsetStrategy.cpp"

#endif
